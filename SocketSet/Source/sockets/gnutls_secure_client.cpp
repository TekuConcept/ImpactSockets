/**
 * Created by TekuConcept on March 7, 2021
 */

#include <errno.h>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <functional>
#include "sockets/gnutls_secure_client.h"
#include "utils/impact_error.h"

#define V(x) std::cout << x << std::endl
#define BEGIN_LOCK(m) { std::lock_guard<std::mutex> lock(m);
#define END_LOCK }

using namespace impact;

namespace impact {

    ssize_t
    gnutls_secure_client_send_callback(
        gnutls_transport_ptr_t __user_data,
        const void*            __data,
        size_t                 __length)
    {
        gnutls_secure_client* client =
            reinterpret_cast<gnutls_secure_client*>(__user_data);
        
        bool result = client->m_base->write(
            std::string((char*)__data, __length));

        if (!result) {
            errno = EAGAIN;
            return -1;
        }
        else return __length;
    }


    ssize_t
    gnutls_secure_client_recv_callback(
        gnutls_transport_ptr_t __user_data,
        void*                  __data,
        size_t                 __length)
    {
        size_t size;
        gnutls_secure_client* client =
            reinterpret_cast<gnutls_secure_client*>(__user_data);

        BEGIN_LOCK(client->m_gnutls_mtx)

        using ready_state = tcp_client_interface::ready_state_t;
        if (client->m_recv_buffer.size() == 0) {
            if (client->m_base->ready_state() == ready_state::DESTROYED ||
                client->m_base->ready_state() == ready_state::WRITE_ONLY)
                return 0;
            else {
                errno = EAGAIN;
                return -1;
            }
        }

        size = std::min(__length, client->m_recv_buffer.size());
        std::memcpy(__data, client->m_recv_buffer.data(), size);
        if (size == client->m_recv_buffer.size())
            client->m_recv_buffer.clear();
        else client->m_recv_buffer = client->m_recv_buffer.substr(size);

        END_LOCK

        return size;
    }

} /* namespace impact */


gnutls_secure_client::gnutls_secure_client(
    tcp_client_t             __base,
    secure_connection_type_t __connection_type,
    gnutls_x509_certificate  __certificate,
    priority_t               __priority)
: m_base(__base),
  m_session(nullptr),
  m_certificate(__certificate),
  m_fast_events(nullptr),
  m_cert_verify_enabled(false),
  m_state(secure_state_t::CLOSED)
{
    /**
     * If client is in read-only or write-only state, then a key
     * exchange can't take place. (current implementation does
     * not yet support socket pairs, which would otherwise allow
     * uni-directional sockets)
     * 
     * If client is destroyed, no further communication can take
     * place. If the client is opening, we might miss the
     * on_connect event and thus no handshake will take place.
     * 
     * For now the only allowed states should be PENDING and OPEN
     */
    if (m_base == nullptr ||
        (m_base->ready_state() != ready_state_t::PENDING &&
         m_base->ready_state() != ready_state_t::OPEN))
        throw impact_error("provided TCP client is not usable");

    m_certificate.set_error_observer(std::bind(
        &gnutls_secure_client::_M_emit_error_message,
        this, std::placeholders::_1));

    _M_init_gnutls_session(__priority, __connection_type);

    this->forward(m_base.get());
    m_base->set_event_observer(this);

    if (m_base->ready_state() == ready_state_t::OPEN) {
        m_state = secure_state_t::OPENING;
        _M_try_handshake();
    }
}


void
gnutls_secure_client::_M_init_gnutls_session(
    priority_t               __priority,
    secure_connection_type_t __connection_type)
{
    int result;
    int session_flags = GNUTLS_NONBLOCK;

    switch (__connection_type) {
    case secure_connection_type_t::CLIENT:
        session_flags |= GNUTLS_CLIENT; break;
    case secure_connection_type_t::SERVER:
        session_flags |= GNUTLS_SERVER; break;
    }

    {
        gnutls_session_t session;
        result = gnutls_init(&session, session_flags);
        if (result < 0) goto error;
        m_session = std::shared_ptr<gnutls_session_int>(session,
        [](gnutls_session_int* p) { gnutls_deinit(p); });

        gnutls_transport_set_push_function(
            m_session.get(),
            gnutls_secure_client_send_callback);
        gnutls_transport_set_pull_function(
            m_session.get(),
            gnutls_secure_client_recv_callback);
        gnutls_transport_set_ptr(m_session.get(), this);
    }

    if (__priority)
        result = gnutls_priority_set(m_session.get(), __priority.get());
    else /* It is recommended to use the default priorities */
        result = gnutls_set_default_priority(m_session.get());
    if (result < 0) goto error;

    /* Put the x509 credentials to the current session */
    result = gnutls_credentials_set(
        m_session.get(), GNUTLS_CRD_CERTIFICATE, m_certificate.get());
    if (result < 0) goto error;

    if (__connection_type == secure_connection_type_t::SERVER)
        gnutls_certificate_server_set_request(
            m_session.get(), GNUTLS_CERT_IGNORE);

    return;

    error:
    throw impact_error(
        std::string(__FUNCTION__) +
        std::string(": ") +
        std::string(gnutls_strerror(result)));
}


void
gnutls_secure_client::_M_emit_error_code(
    std::string __message,
    int         __code)
{
    std::string message = "[security] ";
    message += __message;
    message += std::string(": ");
    message += std::to_string(__code) + std::string(" ");
    message += gnutls_strerror(__code);
    this->on_error(message);
}


inline void
gnutls_secure_client::_M_emit_error_message(std::string __message)
{
    if (m_fast_events)
        m_fast_events->on_error(__message);
    else event_emitter::emit("error", __message);
}


// ----------------------------------------------------------------------------
// secure_client_interface
// ----------------------------------------------------------------------------


const std::string&
gnutls_secure_client::server_name() const
{ return m_server_name; }


void
gnutls_secure_client::server_name(std::string __host)
{
    std::string old_name = std::move(m_server_name);
    m_server_name = __host;
    auto result = gnutls_server_name_set(
        m_session.get(),
        GNUTLS_NAME_DNS,
        m_server_name.c_str(),
        m_server_name.size());
    if (result < 0) {
        m_server_name = std::move(old_name);
        _M_emit_error_code(__FUNCTION__, result);
    }
    else _M_set_verify_cert();
}


bool
gnutls_secure_client::cert_verify_enabled() const
{ return m_cert_verify_enabled; }


void
gnutls_secure_client::cert_verify_enabled(bool __enabled)
{
    m_cert_verify_enabled = __enabled;
    _M_set_verify_cert();
}


inline void
gnutls_secure_client::_M_set_verify_cert()
{
    const char* host = NULL;
    if (m_server_name.size() > 0 && m_cert_verify_enabled)
        host = m_server_name.c_str();
    gnutls_session_set_verify_cert(m_session.get(), host, /*flags=*/0);
}


// ----------------------------------------------------------------------------
// secure_x509_certificate_interface
// ----------------------------------------------------------------------------


void
gnutls_secure_client::set_x509_trust(
    std::string     __trust,
    secure_format_t __format)
{ m_certificate.set_x509_trust(__trust, __format); }


void
gnutls_secure_client::set_x509_cert_revoke_list(
    std::string     __crl,
    secure_format_t __format)
{ m_certificate.set_x509_cert_revoke_list(__crl, __format); }


void
gnutls_secure_client::set_x509_ocsp_request_file(
    std::string __ocsp_request_file,
    size_t      __index)
{ m_certificate.set_x509_ocsp_request_file(__ocsp_request_file, __index); }


void
gnutls_secure_client::set_x509_credentials(
    std::string     __key,
    std::string     __certificate,
    secure_format_t __format)
{ m_certificate.set_x509_credentials(__key, __certificate, __format); }


// ----------------------------------------------------------------------------
// tcp_client_interface
// ----------------------------------------------------------------------------


tcp_address_t
gnutls_secure_client::address() const
{ return m_base->address(); }


size_t
gnutls_secure_client::bytes_read() const
{ return m_base->bytes_read(); }


size_t
gnutls_secure_client::bytes_written() const
{ return m_base->bytes_written(); }


bool
gnutls_secure_client::connecting() const
{ return m_base->connecting(); }


bool
gnutls_secure_client::destroyed() const
{ return m_base->destroyed(); }


std::string
gnutls_secure_client::local_address() const
{ return m_base->local_address(); }


unsigned short
gnutls_secure_client::local_port() const
{ return m_base->local_port(); }


bool
gnutls_secure_client::pending() const
{ return m_base->pending(); }


std::string
gnutls_secure_client::remote_address() const
{ return m_base->remote_address(); }


address_family
gnutls_secure_client::remote_family() const
{ return m_base->remote_family(); }


unsigned short
gnutls_secure_client::remote_port() const
{ return m_base->remote_port(); }


size_t
gnutls_secure_client::timeout() const
{ return m_base->timeout(); }


tcp_client_interface::ready_state_t
gnutls_secure_client::ready_state() const
{ return m_base->ready_state(); }


tcp_client_interface*
gnutls_secure_client::connect(
    std::string               __path,
    event_emitter::callback_t __cb)
{
    m_base->connect(__path, __cb);
    return this;
}


tcp_client_interface*
gnutls_secure_client::connect(
    unsigned short            __port,
    std::string               __host,
    event_emitter::callback_t __cb)
{
    m_base->connect(__port, __host, __cb);
    return this;
}


tcp_client_interface*
gnutls_secure_client::destroy(std::string __error)
{
    _M_destroy();
    if (__error.size() > 0)
        _M_emit_error_message(__error);
    return this;
}


void
gnutls_secure_client::_M_destroy()
{
    int result = 0;

    if ((m_state == secure_state_t::OPEN ||
         m_state == secure_state_t::CLOSING) &&
        (m_base->ready_state() == ready_state_t::OPEN ||
         m_base->ready_state() == ready_state_t::WRITE_ONLY))
        result = gnutls_bye(m_session.get(), GNUTLS_SHUT_RDWR);

    if (result == 0) m_state = secure_state_t::CLOSED;
    else if (result == GNUTLS_E_AGAIN || result == GNUTLS_E_INTERRUPTED)
    { m_state = secure_state_t::CLOSING; }

    if (m_state == secure_state_t::CLOSED)
        m_base->destroy();
    else if (m_state != secure_state_t::CLOSING && result < 0) {
        // forcefully closed
        m_base->destroy();
        m_state = secure_state_t::CLOSED;
        if (result < 0)
            _M_emit_error_code(__FUNCTION__, result);
    }
}


tcp_client_interface*
gnutls_secure_client::end(
    std::string               __data,
    std::string               __encoding,
    event_emitter::callback_t __cb)
{
    event_emitter::once("end", __cb);
    write(__data, __encoding);
    _M_end();
    return this;
}


void
gnutls_secure_client::_M_end()
{
    int result = 0;
    if ((m_state == secure_state_t::OPEN ||
         m_state == secure_state_t::ENDING) &&
        (m_base->ready_state() == ready_state_t::OPEN ||
         m_base->ready_state() == ready_state_t::WRITE_ONLY))
        result = gnutls_bye(m_session.get(), GNUTLS_SHUT_WR);

    if (result == 0) m_state = secure_state_t::ENDED;
    else if (result == GNUTLS_E_AGAIN || result == GNUTLS_E_INTERRUPTED)
    { m_state = secure_state_t::ENDING; }

    if (m_state == secure_state_t::ENDED)
        m_base->end();
    else if (m_state != secure_state_t::ENDING && result < 0) {
        // forcefully ended
        m_base->end();
        m_state = secure_state_t::ENDED;
        if (result < 0)
            _M_emit_error_code(__FUNCTION__, result);
    }
}


tcp_client_interface*
gnutls_secure_client::pause()
{
    m_base->pause();
    return this;
}


tcp_client_interface*
gnutls_secure_client::resume()
{
    m_base->resume();
    return this;
}


tcp_client_interface*
gnutls_secure_client::set_encoding(std::string /* __encoding */)
{ return this; }


tcp_client_interface*
gnutls_secure_client::set_keep_alive(
    bool         __enable,
    unsigned int __initial_delay)
{
    m_base->set_keep_alive(__enable, __initial_delay);
    return this;
}


tcp_client_interface*
gnutls_secure_client::set_no_delay(bool __no_delay)
{
    m_base->set_no_delay(__no_delay);
    return this;
}


tcp_client_interface*
gnutls_secure_client::set_timeout(
    unsigned int              __timeout,
    event_emitter::callback_t __cb)
{
    m_base->set_timeout(__timeout, __cb);
    return this;
}


bool
gnutls_secure_client::write(
    std::string               __data,
    std::string             /*__encoding*/,
    event_emitter::callback_t __cb)
{
    if (__data.size() == 0) {
        if (__cb) __cb({});
        return true;
    }
    auto result = gnutls_record_send(
        m_session.get(), &__data[0], __data.size());
    if (result < 0) {
        if (result != GNUTLS_E_AGAIN) {
            impact_error error(__FUNCTION__);
            _M_emit_error_code(error.what(), result);
        }
    }
    else { if (__cb) __cb({}); }
    return result < 0;
}


void
gnutls_secure_client::set_event_observer(
    tcp_client_observer_interface* __fast_events)
{ m_fast_events = __fast_events; }


// ----------------------------------------------------------------------------
// tcp_client_observer_interface
// ----------------------------------------------------------------------------


void
gnutls_secure_client::on_close(bool __transmission_error)
{
    if (m_fast_events)
        m_fast_events->on_close(__transmission_error);
    else event_emitter::emit("close");
}


void
gnutls_secure_client::on_connect()
{
    if (m_fast_events)
        m_fast_events->on_connect();
    else event_emitter::emit("connect");
    m_state = secure_state_t::OPENING;
    _M_try_handshake();
}


void
gnutls_secure_client::on_data(std::string& __data)
{
    std::string block_buffer(256, '\0');
    size_t buffer_size;

    BEGIN_LOCK(m_gnutls_mtx)
    m_recv_buffer.append(__data);
    buffer_size = m_recv_buffer.size();
    END_LOCK

    change_state:
    switch (m_state) {
    case secure_state_t::OPEN: {
        while (buffer_size > 0) {
            ssize_t result = gnutls_record_recv(
                m_session.get(), &block_buffer[0], block_buffer.size());
            BEGIN_LOCK(m_gnutls_mtx)
            buffer_size = m_recv_buffer.size();
            END_LOCK
            if (result == GNUTLS_E_REHANDSHAKE) {
                m_state = secure_state_t::OPENING;
                goto change_state;
            }
            else if (result < 0 && gnutls_error_is_fatal(result) == 0)
            { if (buffer_size == 0) return; }
            else if (result < 0)
                _M_fatal_error(result);
            else {
                std::string data = block_buffer.substr(0, result);
                if (m_fast_events) m_fast_events->on_data(data);
                else event_emitter::emit("data", data);
            }
            // TODO: process GNUTLS_E_WARNING_ALERT_RECEIVED
        }
    } break;
    case secure_state_t::OPENING: {
        while (m_state == secure_state_t::OPENING && buffer_size > 0) {
            _M_try_handshake();
            BEGIN_LOCK(m_gnutls_mtx)
            buffer_size = m_recv_buffer.size();
            END_LOCK
        }
        if (m_state == secure_state_t::OPEN)
            goto change_state;
    } break;
    case secure_state_t::ENDING: {
        while (m_state == secure_state_t::ENDING && buffer_size > 0) {
            _M_end();
            BEGIN_LOCK(m_gnutls_mtx)
            buffer_size = m_recv_buffer.size();
            END_LOCK
        }
        if (m_state == secure_state_t::ENDED && buffer_size > 0)
            goto change_state;
    } break;
    case secure_state_t::CLOSING: {
        while (m_state == secure_state_t::CLOSING && buffer_size > 0) {
            _M_destroy();
            BEGIN_LOCK(m_gnutls_mtx)
            buffer_size = m_recv_buffer.size();
            END_LOCK
        }
    } break;
    case secure_state_t::ENDED: {
        ssize_t result;
        bool again = false;
        //
        // return any data that is received until EOF
        //
        do {
            result = gnutls_record_recv(
                m_session.get(), &block_buffer[0], block_buffer.size());
            BEGIN_LOCK(m_gnutls_mtx)
            buffer_size = m_recv_buffer.size();
            END_LOCK
            if (result == 0) _M_destroy();
            else if (result > 0) {
                std::string data = block_buffer.substr(0, result);
                if (m_fast_events) m_fast_events->on_data(data);
                else event_emitter::emit("data", data);
            }
            again = (result > 0) || ((
                result == GNUTLS_E_AGAIN ||
                result == GNUTLS_E_INTERRUPTED
            ) && buffer_size > 0);
        } while (again);
    } break;
    case secure_state_t::CLOSED: {
        // session is closed, any new data is invalid
        BEGIN_LOCK(m_gnutls_mtx)
        m_recv_buffer.clear();
        END_LOCK
    } break;
    }
}


void
gnutls_secure_client::_M_fatal_error(int __code)
{
    std::string message = "[security] [fetal] ";
    message += std::string(gnutls_strerror_name(__code));
    message += " ";
    message += std::string(gnutls_strerror(__code));
    message += "\n";
    if (__code == GNUTLS_E_CERTIFICATE_VERIFICATION_ERROR) {
        gnutls_datum_t info;
        auto cert_type = gnutls_certificate_type_get(m_session.get());
        auto cert_status =
            gnutls_session_get_verify_cert_status(m_session.get());
        auto result = gnutls_certificate_verification_status_print(
            cert_status, cert_type, &info, 0);
        if (result < 0)
            message += "no verification status vailable - unknown";
        else {
            message += std::string((char*)info.data, info.size);
            gnutls_free(info.data);
        }
    }
    m_recv_buffer.clear();
    m_base->destroy(message);
}


void
gnutls_secure_client::_M_try_handshake()
{
    int status = gnutls_handshake(m_session.get());
    if (status == GNUTLS_E_SUCCESS) {
        m_state = secure_state_t::OPEN;
        if (m_fast_events)
            m_fast_events->on_ready();
        else {
            event_emitter::emit("secure-connect");
            event_emitter::emit("ready");
        }
        return;
    }
    // TODO: process GNUTLS_E_WARNING_ALERT_RECEIVED
    // TODO: process GNUTLS_E_GOT_APPLICATION_DATA
    if (gnutls_error_is_fatal(status) != 0)
        _M_fatal_error(status);
}


void
gnutls_secure_client::on_end()
{
    if (m_fast_events)
        m_fast_events->on_end();
    else event_emitter::emit("end");
}


void
gnutls_secure_client::on_error(const std::string& __message)
{ _M_emit_error_message(__message); }


void
gnutls_secure_client::on_lookup(
    std::string&   __error,
    std::string&   __address,
    address_family __family,
    std::string&   __host)
{
    if (m_fast_events)
        m_fast_events->on_lookup(__error, __address, __family, __host);
    else event_emitter::emit("lookup", __error, __address, __family, __host);
}


void
gnutls_secure_client::on_ready()
{ /* reserve ready event for when the handshake completes */ }


void
gnutls_secure_client::on_timeout()
{
    if (m_fast_events)
        m_fast_events->on_timeout();
    else event_emitter::emit("timeout");
}
