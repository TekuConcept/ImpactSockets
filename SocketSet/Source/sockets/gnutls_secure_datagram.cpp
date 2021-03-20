/**
 * Created by TekuConcept on March 16, 2021
 */

#include <iostream>
#include <iomanip>
#include <cstring>
#include "sockets/gnutls_secure_datagram.h"
#include "utils/environment.h"

using namespace impact;

#define V(x) std::cout << x << std::endl
#define BEGIN_LOCK(m) { std::lock_guard<std::mutex> lock(m);
#define END_LOCK(m) }
#define LOOPBACK "loopback"


gnutls_secure_datagram::gnutls_secure_datagram(udp_socket_t __base)
: m_base(__base),
  m_fast_events(nullptr),
  m_server_enabled(false),
  m_priority_cache(nullptr),
  m_cookie_key(nullptr),
  m_mtu(1400),
  m_state(secure_state_t::OPEN)
{
    if (m_base == nullptr)
        throw impact_error("provided UDP socket is not usable");

    #if GNUTLS_VERSION_NUMBER < 0x030600
    int result = gnutls_certificate_set_known_dh_params(
        m_server_certificate.get(), GNUTLS_SEC_PARAM_MEDIUM);
    if (result < 0) throw impact_error(gnutls_strerror(result));
    #endif

    _M_init_server_info();

    this->forward(m_base.get());
    m_base->set_event_observer(this);
}


void
gnutls_secure_datagram::_M_emit_error_code(
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
gnutls_secure_datagram::_M_emit_error_message(std::string __message)
{
    if (m_fast_events)
        m_fast_events->on_error(__message);
    else event_emitter::emit("error", __message);
}


void
gnutls_secure_datagram::_M_fatal_error(
    route_t __route,
    int     __code)
{
    V(__FUNCTION__);
    impact_error error("");
    std::string message = "[security] [fetal] [";
    message += __route->address.address + std::string(":");
    message += std::to_string(__route->address.port) + std::string("] ");
    message += std::string(gnutls_strerror_name(__code));
    message += " ";
    message += std::string(gnutls_strerror(__code));
    message += "\n";
    message += error.what();
    if (__code == GNUTLS_E_CERTIFICATE_VERIFICATION_ERROR) {
        gnutls_datum_t info;
        auto cert_type = gnutls_certificate_type_get(__route->session.get());
        auto cert_status =
            gnutls_session_get_verify_cert_status(__route->session.get());
        auto result = gnutls_certificate_verification_status_print(
            cert_status, cert_type, &info, 0);
        if (result < 0)
            message += "no verification status vailable - unknown";
        else {
            message += std::string((char*)info.data, info.size);
            gnutls_free(info.data);
        }
    }
    // m_recv_buffer.clear();
    // m_base->destroy(message);
    _M_emit_error_message(message);
}


gnutls_secure_datagram::route_t
gnutls_secure_datagram::_M_find_route(udp_address_t __address)
{
    V(__FUNCTION__);
    route_t result = nullptr;
    BEGIN_LOCK(m_route_mtx)
    auto route = m_routes.find(__address);
    if (route != m_routes.end())
        result = route->second;
    END_LOCK(m_route_mtx)
    if (result == nullptr)
        this->on_error("route doesn't exist");
    return result;
}


void
gnutls_secure_datagram::_M_init_server_info()
{
    V(__FUNCTION__);
    int result;
    
    { // priority_cache
        gnutls_priority_t priority_cache;
        #if GNUTLS_VERSION_NUMBER >= 0x030603
        result = gnutls_priority_init2(
            &priority_cache,
            "%SERVER_PRECEDENCE",
            NULL,
            GNUTLS_PRIORITY_INIT_DEF_APPEND);
        #else
        result = gnutls_priority_init(
            &priority_cache,
            "NORMAL:-VERS-TLS-ALL:+VERS-DTLS1.0:%SERVER_PRECEDENCE",
            NULL);
        #endif
        if (result < 0) goto error;
        m_priority_cache = priority_t(
            priority_cache,
            [](gnutls_priority_st* p)
            { gnutls_priority_deinit(p); });
    }

    { // cookie key
        m_cookie_key = std::shared_ptr<gnutls_datum_t>(
            new gnutls_datum_t{ .data = NULL, .size = 0 },
            [](gnutls_datum_t* p) {
                if (p->data != NULL)
                    gnutls_free(p->data);
                delete p;
            }
        );
        result = gnutls_key_generate(m_cookie_key.get(), GNUTLS_COOKIE_KEY_SIZE);
        if (result < 0) goto error;
    }

    return;
    error:
    throw impact_error(
        std::string(__FUNCTION__) +
        std::string(": ") +
        std::string(gnutls_strerror(result)));
}


void
gnutls_secure_datagram::_M_init_loopback()
{
    V(__FUNCTION__);
    auto address = m_base->address();
    if (address.family != address_family::INET &&
        address.family != address_family::INET6) return;

    udp_address_t loopback_address;
    loopback_address.address = LOOPBACK;
    loopback_address.port    = address.port;
    loopback_address.family  = address_family::UNSPECIFIED;

    route_t server = std::make_shared<route_context>();
    server->address             = address;
    server->state               = secure_state_t::OPENING;
    server->cert_verify_enabled = false;
    server->server_name         = "";
    server->parent              = this;
    server->certificate         = m_server_certificate;

    auto prestate = std::make_shared<gnutls_dtls_prestate_st>();
    memset(prestate.get(), 0, sizeof(gnutls_dtls_prestate_st));
    _M_init_gnutls_session(
        server,
        secure_connection_type_t::SERVER,
        m_priority_cache,
        prestate);

    // set special-case callback
    gnutls_transport_set_push_function(
        server->session.get(), _S_on_loopback_server_send_callback);

    BEGIN_LOCK(m_route_mtx)
    m_routes[loopback_address] = server;
    END_LOCK(m_route_mtx)
}


void
gnutls_secure_datagram::_M_init_gnutls_session(
    route_t                  __route,
    secure_connection_type_t __connection_type,
    priority_t               __priority_cache,
    prestate_t               __prestate)
{
    V(__FUNCTION__);
    int result;
    int session_flags = GNUTLS_DATAGRAM | GNUTLS_NONBLOCK;

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
        __route->session = std::shared_ptr<gnutls_session_int>(session,
        [](gnutls_session_int* p) { gnutls_deinit(p); });

        gnutls_transport_set_push_function(
            __route->session.get(), _S_on_send_callback);
        gnutls_transport_set_pull_function(
            __route->session.get(), _S_on_recv_callback);
        gnutls_transport_set_pull_timeout_function(
            __route->session.get(), _S_on_recv_timeout_callback);
        gnutls_transport_set_ptr(__route->session.get(), __route.get());
        gnutls_dtls_set_timeouts(__route->session.get(), 61000, 60000);
    }

    if (__priority_cache)
        result = gnutls_priority_set(
            __route->session.get(), __priority_cache.get());
    else /* It is recommended to use the default priorities */
        result = gnutls_set_default_priority(__route->session.get());
    if (result < 0) goto error;

    /* Put the x509 credentials to the current session */
    result = gnutls_credentials_set(
        __route->session.get(),
        GNUTLS_CRD_CERTIFICATE,
        __route->certificate.get());
    if (result < 0) goto error;

    gnutls_dtls_set_mtu(__route->session.get(), m_mtu);

    if (__connection_type == secure_connection_type_t::SERVER) {
        __route->prestate = __prestate;
        gnutls_dtls_prestate_set(
            __route->session.get(),
            __route->prestate.get());
    }

    return;

    error:
    throw impact_error(
        std::string(__FUNCTION__) +
        std::string(": ") +
        std::string(gnutls_strerror(result)));
}


inline void
gnutls_secure_datagram::_M_set_verify_cert(route_t __route)
{
    const char* host = NULL;
    if (__route->server_name.size() > 0 &&
        __route->cert_verify_enabled)
        host = __route->server_name.c_str();
    gnutls_session_set_verify_cert(
        __route->session.get(), host, /*flags=*/0);
}


inline gnutls_secure_datagram::route_t
gnutls_secure_datagram::_M_create_server_agent(
    std::string   __data,
    udp_address_t __address)
{
    if (__data.size() == 0) return nullptr;

    auto prestate = std::make_shared<gnutls_dtls_prestate_st>();
    memset(prestate.get(), 0, sizeof(gnutls_dtls_prestate_st));

    // lock m_routes_mtx outside function call
    route_t connection = std::make_shared<route_context>();
    connection->address             = __address;
    connection->state               = secure_state_t::OPENING;
    connection->cert_verify_enabled = false;
    connection->server_name         = "";
    connection->parent              = this;

    // attempt to prevent DoS
    int result;
    std::string cookie_data =
        __address.address +
        std::string(":") +
        std::to_string(__address.port);
    result = gnutls_dtls_cookie_verify(
        m_cookie_key.get(),
        &cookie_data[0], cookie_data.size(),
        &__data[0], __data.size(),
        prestate.get());
    if (result < 0) {
        gnutls_dtls_cookie_send(
            m_cookie_key.get(),
            &cookie_data[0], cookie_data.size(),
            prestate.get(),
            (gnutls_transport_ptr_t)connection.get(),
            _S_on_send_callback);
        // ignore potential error code from gnutls_dtls_cookie_send
        return nullptr;
    }

    try {
        _M_init_gnutls_session(
            connection,
            secure_connection_type_t::SERVER,
            m_priority_cache,
            prestate);
    }
    catch (impact_error& error) {
        _M_emit_error_message(error.what());
        return nullptr;
    }

    BEGIN_LOCK(m_route_mtx)
    m_routes[__address] = connection;
    END_LOCK(m_route_mtx)
    return connection;
}


void
gnutls_secure_datagram::_M_try_handshake(route_t __route)
{
    V(__FUNCTION__);
    int status = gnutls_handshake(__route->session.get());
    if (status == GNUTLS_E_SUCCESS) {
        __route->state = secure_state_t::OPEN;
        event_emitter::emit("secure-connect", __route->address);
        event_emitter::emit("ready", __route->address);
        return;
    }
    V(__FUNCTION__ << ": " << status);
    // TODO: process GNUTLS_E_WARNING_ALERT_RECEIVED
    // TODO: process GNUTLS_E_GOT_APPLICATION_DATA
    if (gnutls_error_is_fatal(status) != 0)
        _M_fatal_error(__route, status);
}


void
gnutls_secure_datagram::_M_end(route_t __route)
{
    int result = 0;
    if (__route->state == secure_state_t::OPEN ||
        __route->state == secure_state_t::ENDING)
        result = gnutls_bye(__route->session.get(), GNUTLS_SHUT_WR);

    if (result == 0) __route->state = secure_state_t::ENDED;
    else if (result == GNUTLS_E_AGAIN || result == GNUTLS_E_INTERRUPTED)
    { __route->state = secure_state_t::ENDING; }

    if (__route->state == secure_state_t::ENDED ||
        (__route->state != secure_state_t::ENDING && result < 0)) {
        // forcefully ended
        __route->state = secure_state_t::ENDED;
        event_emitter::emit("end", __route->address);
        if (result < 0)
            _M_emit_error_code(__FUNCTION__, result);
    }
}


void
gnutls_secure_datagram::_M_destroy(route_t __route)
{
    int result = 0;

    if (__route->state == secure_state_t::OPEN ||
        __route->state == secure_state_t::CLOSING)
        result = gnutls_bye(__route->session.get(), GNUTLS_SHUT_RDWR);

    if (result == 0) __route->state = secure_state_t::CLOSED;
    else if (result == GNUTLS_E_AGAIN || result == GNUTLS_E_INTERRUPTED)
    { __route->state = secure_state_t::CLOSING; }

    if (__route->state == secure_state_t::CLOSED ||
        (__route->state != secure_state_t::CLOSING && result < 0)) {
        // forcefully closed
        __route->state = secure_state_t::CLOSED;
        BEGIN_LOCK(m_route_mtx)
        auto route = m_routes.find(__route->address);
        if (route != m_routes.end())
            m_routes.erase(route);
        END_LOCK(m_route_mtx)
        event_emitter::emit("destroy", __route->address);
        if (result < 0)
            _M_emit_error_code(__FUNCTION__, result);
    }
}


void
gnutls_secure_datagram::_S_process_message(
    std::string __data,
    route_t     __connection)
{
    V(__FUNCTION__);
    auto parent = __connection->parent;
    std::string block_buffer(parent->m_mtu, '\0');
    size_t buffer_size;

    __connection->recv_buffer = __data;
    buffer_size = __data.size();

    change_state:
    switch (__connection->state) {
    case secure_state_t::OPEN: {
        V("-- OPEN --");
        while (buffer_size > 0) {
            ssize_t result = gnutls_record_recv(
                __connection->session.get(),
                &block_buffer[0],
                block_buffer.size());
            buffer_size = __connection->recv_buffer.size();
            if (result == GNUTLS_E_REHANDSHAKE) {
                __connection->state = secure_state_t::OPENING;
                goto change_state;
            }
            else if (result < 0 && gnutls_error_is_fatal(result) == 0)
            { if (buffer_size == 0) return; }
            else if (result < 0)
                parent->_M_fatal_error(__connection, result);
            else {
                std::string data = block_buffer.substr(0, result);
                if (parent->m_fast_events)
                    parent->m_fast_events->on_message(
                        data, __connection->address);
                else parent->emit("data", data, __connection->address);
            }
            // TODO: process GNUTLS_E_WARNING_ALERT_RECEIVED
        }
    } break;
    case secure_state_t::OPENING: {
        V("-- OPENING --");
        while (__connection->state == secure_state_t::OPENING && buffer_size > 0) {
            V("[opening] buffer_size: " << buffer_size);
            V("trying connection handshake");
            parent->_M_try_handshake(__connection);
            buffer_size = __connection->recv_buffer.size();
        }
        if (__connection->state == secure_state_t::OPEN)
            goto change_state;
    } break;
    case secure_state_t::ENDING: {
        V("-- ENDING --");
        while (__connection->state == secure_state_t::ENDING && buffer_size > 0) {
            parent->_M_end(__connection);
            buffer_size = __connection->recv_buffer.size();
        }
        if (__connection->state == secure_state_t::ENDED && buffer_size > 0)
            goto change_state;
    } break;
    case secure_state_t::CLOSING: {
        V("-- CLOSING --");
        while (__connection->state == secure_state_t::CLOSING && buffer_size > 0) {
            parent->_M_destroy(__connection);
            buffer_size = __connection->recv_buffer.size();
        }
        if (__connection->state == secure_state_t::CLOSED)
            goto change_state;
    } break;
    case secure_state_t::ENDED: {
        V("-- ENDED --");
        ssize_t result;
        bool again = false;
        //
        // return any data that is received until EOF
        //
        do {
            result = gnutls_record_recv(
                __connection->session.get(),
                &block_buffer[0], block_buffer.size());
            buffer_size = __connection->recv_buffer.size();
            if (result == 0) {
                parent->_M_destroy(__connection);
                if (__connection->state == secure_state_t::CLOSED)
                    goto change_state;
            }
            else if (result > 0) {
                std::string data = block_buffer.substr(0, result);
                if (parent->m_fast_events)
                    parent->m_fast_events->on_message(
                        data, __connection->address);
                else parent->emit("data", data, __connection->address);
            }
            again = (result > 0) || ((
                result == GNUTLS_E_AGAIN ||
                result == GNUTLS_E_INTERRUPTED
            ) && buffer_size > 0);
        } while (again);
    } break;
    case secure_state_t::CLOSED: {
        V("-- CLOSED --");
        // session is closed, any new data is invalid
        __connection->recv_buffer.clear();
    } break;
    }
}


ssize_t
gnutls_secure_datagram::_S_on_send_callback(
    gnutls_transport_ptr_t __user_data,
    const void*            __buffer,
    size_t                 __size)
{
    V("=== " << __FUNCTION__ << " ===");
    gnutls_secure_datagram::route_context* route =
        reinterpret_cast<gnutls_secure_datagram::route_context*>(__user_data);
    V("ptr: " << __user_data);
    V("sending " << __size << " to "
        << route->address.address << ":" << route->address.port);
    route->parent->m_base->send(
        std::string((char*)__buffer, __size),
        route->address.port,
        route->address.address);
    return __size;
}


ssize_t
gnutls_secure_datagram::_S_on_recv_callback(
    gnutls_transport_ptr_t __user_data,
    void*                  __buffer,
    size_t                 __size)
{
    V(__FUNCTION__);
    size_t size;
    gnutls_secure_datagram::route_context* route =
        reinterpret_cast<gnutls_secure_datagram::route_context*>(__user_data);
    V("recv: " << route->address.address << ":" << route->address.port
        << " size: " << route->recv_buffer.size());
    if (route->recv_buffer.size() == 0) {
        V("recv EAGAIN " << route->recv_buffer.size());
        errno = EAGAIN;
        return -1;
    }

    size = std::min(__size, route->recv_buffer.size());
    V("comp(" << __size << ", " << route->recv_buffer.size() << ") -> " << size);
    std::memcpy(__buffer, route->recv_buffer.c_str(), size);

    V("------------------------------------------------");
    std::cout << std::hex << std::setfill('0');
    for (size_t i = 0; i < size; i++) {
        if (i != 0 && i % 16 == 0) std::cout << std::endl;
        std::cout << std::setw(2) << (int)(0xFF & ((unsigned char*)__buffer)[i]) << " ";
    }
    std::cout << std::dec << std::setfill(' ') << std::endl;
    V("================================================");

    if (size == route->recv_buffer.size()) {
        V("buff emptied");
        route->recv_buffer.clear();
    }
    else {
        route->recv_buffer = route->recv_buffer.substr(size);
        V("buff partial read; remaining: " << route->recv_buffer.size());
    }
    return size;
}


int
gnutls_secure_datagram::_S_on_recv_timeout_callback(
    gnutls_transport_ptr_t __user_data,
    unsigned int           __ms)
{
    V(__FUNCTION__ << " ms: " << __ms);
    gnutls_secure_datagram::route_context* route =
        reinterpret_cast<gnutls_secure_datagram::route_context*>(__user_data);
    if (route->recv_buffer.size() == 0) {
        errno = EAGAIN;
        return -1;
    }
    else return route->recv_buffer.size();
}


ssize_t
gnutls_secure_datagram::_S_on_loopback_server_send_callback(
    gnutls_transport_ptr_t __user_data,
    const void*            __buffer,
    size_t                 __size)
{
    V("=== " << __FUNCTION__ << " ===");
    using route_context = gnutls_secure_datagram::route_context;
    route_context* server = reinterpret_cast<route_context*>(__user_data);
    V(__FUNCTION__ << "| " << server->address.address << ":" << server->address.port);
    auto loopback_address = server->address;
    loopback_address.address = LOOPBACK;
    server->parent->on_message(
        std::string((const char*)__buffer, __size),
        loopback_address);
    return __size;
}


// ----------------------------------------------------------------------------
// secure_datagram_interface
// ----------------------------------------------------------------------------


size_t
gnutls_secure_datagram::mtu() const
{ return m_mtu; }


void
gnutls_secure_datagram::mtu(size_t __value)
{
    m_mtu = __value;
    BEGIN_LOCK(m_route_mtx)
    for (auto& route : m_routes)
        gnutls_dtls_set_mtu(route.second->session.get(), m_mtu);
    END_LOCK(m_route_mtx)
}


void
gnutls_secure_datagram::create(udp_address_t __address)
{
    V(__FUNCTION__);
    std::string message;
    BEGIN_LOCK(m_route_mtx)
    auto route = m_routes.find(__address);
    if (route != m_routes.end()) return;
    
    route_t connection              = std::make_shared<route_context>();
    connection->address             = __address;
    connection->state               = secure_state_t::CLOSED;
    connection->cert_verify_enabled = false;
    connection->server_name         = "";
    connection->parent              = this;

    try {
        _M_init_gnutls_session(
            connection,
            secure_connection_type_t::CLIENT,
            nullptr, nullptr);
    }
    catch (impact_error& error) {
        message = error.what();
        goto error;
    }

    m_routes[__address] = connection;
    END_LOCK(m_route_mtx)

    { // run this outside locked scope to avoid deadlocking
        auto loopback_address = m_base->address();
        if ((__address.address == loopback_address.address) &&
            (__address.port    == loopback_address.port)) {
            try { _M_init_loopback(); }
            catch (impact_error& error)
            { message = error.what(); goto error; }
        }
    }

    return;

    error:
    _M_emit_error_message(message);
}


void
gnutls_secure_datagram::begin(udp_address_t __address)
{
    V(__FUNCTION__);
    auto connection = _M_find_route(__address);
    if (connection == nullptr) return;
    if (connection->state == secure_state_t::CLOSED ||
        connection->state == secure_state_t::OPENING) {
        connection->state = secure_state_t::OPENING;
        V("trying client handshake");
        _M_try_handshake(connection);
    }
}


void
gnutls_secure_datagram::end(udp_address_t __address)
{
    V(__FUNCTION__);
    auto connection = _M_find_route(__address);
    if (connection != nullptr)
        _M_end(connection);
}


void
gnutls_secure_datagram::destroy(udp_address_t __address)
{
    V(__FUNCTION__);
    auto connection = _M_find_route(__address);
    if (connection != nullptr)
        _M_destroy(connection);
}


const std::string&
gnutls_secure_datagram::server_name(udp_address_t __address)
{
    route_t result = _M_find_route(__address);
    if (result != nullptr)
        return result->server_name;
    else return m_string_placeholder;
}


void
gnutls_secure_datagram::server_name(
    udp_address_t __address,
    std::string   __host)
{
    route_t connection = _M_find_route(__address);
    if (connection != nullptr) {
        std::string old_name = std::move(connection->server_name);
        connection->server_name = __host;
        auto result = gnutls_server_name_set(
            connection->session.get(),
            GNUTLS_NAME_DNS,
            connection->server_name.c_str(),
            connection->server_name.size());
        if (result < 0) {
            connection->server_name = std::move(old_name);
            _M_emit_error_code(__FUNCTION__, result);
        }
        else _M_set_verify_cert(connection);
    }
}


bool
gnutls_secure_datagram::cert_verify_enabled(udp_address_t __address)
{
    route_t result = _M_find_route(__address);
    if (result != nullptr)
        return result->cert_verify_enabled;
    else return false;
}


void
gnutls_secure_datagram::cert_verify_enabled(
    udp_address_t __address,
    bool          __enabled)
{
    route_t result = _M_find_route(__address);
    if (result == nullptr) {
        result->cert_verify_enabled = __enabled;
        _M_set_verify_cert(result);
    }
}


void
gnutls_secure_datagram::set_x509_client_credentials(
    udp_address_t   __address,
    std::string     __key,
    std::string     __certificate,
    secure_format_t __format)
{
    V(__FUNCTION__);
    auto connection = _M_find_route(__address);
    if (connection != nullptr) {
        try {
            connection->certificate.set_x509_credentials(
                __key, __certificate, __format);
        }
        catch (impact_error& error)
        { _M_emit_error_message(error.what()); }
    }
}


void
gnutls_secure_datagram::enable_server(bool __enabled)
{ m_server_enabled = __enabled; }


// ----------------------------------------------------------------------------
// secure_x509_certificate_interface
// ----------------------------------------------------------------------------


void
gnutls_secure_datagram::set_x509_trust(
    std::string     __trust,
    secure_format_t __format)
{ m_server_certificate.set_x509_trust(__trust, __format); }


void
gnutls_secure_datagram::set_x509_cert_revoke_list(
    std::string     __crl,
    secure_format_t __format)
{ m_server_certificate.set_x509_cert_revoke_list(__crl, __format); }


void
gnutls_secure_datagram::set_x509_ocsp_request_file(
    std::string __ocsp_request_file,
    size_t      __index)
{ m_server_certificate.set_x509_ocsp_request_file(__ocsp_request_file, __index); }


void
gnutls_secure_datagram::set_x509_credentials(
    std::string     __key,
    std::string     __certificate,
    secure_format_t __format)
{ m_server_certificate.set_x509_credentials(__key, __certificate, __format); }


// ----------------------------------------------------------------------------
// udp_socket_interface
// ----------------------------------------------------------------------------


int
gnutls_secure_datagram::descriptor() const
{ return m_base->descriptor(); }


udp_address_t
gnutls_secure_datagram::address() const
{ return m_base->address(); }


udp_address_t
gnutls_secure_datagram::remote_address() const
{ return m_base->remote_address(); }


void
gnutls_secure_datagram::add_membership(
    std::string __address,
    std::string __interface)
{ m_base->add_membership(__address, __interface); }


void
gnutls_secure_datagram::drop_membership(
    std::string __address,
    std::string __interface)
{ m_base->drop_membership(__address, __interface); }


void
gnutls_secure_datagram::bind(
    unsigned short            __port,
    std::string               __address,
    event_emitter::callback_t __cb)
{ m_base->bind(__port, __address, __cb); }


void
gnutls_secure_datagram::close(event_emitter::callback_t __cb)
{
    event_emitter::once("close", __cb);
    state_change:
    switch (m_state) {
    case secure_state_t::OPENING:
        m_state = secure_state_t::OPEN;
        goto state_change;
    case secure_state_t::OPEN: {
        m_state = secure_state_t::CLOSING;
        std::vector<route_map_t::value_type> routes;
        BEGIN_LOCK(m_route_mtx)
        routes = std::vector<route_map_t::value_type>(
            m_routes.begin(), m_routes.end());
        END_LOCK(m_route_mtx)
        for (auto& route : routes)
            _M_destroy(route.second);
        BEGIN_LOCK(m_route_mtx)
        if (m_routes.size() == 0)
            goto state_change;
        END_LOCK(m_route_mtx)
    } break;
    // force-close
    case secure_state_t::ENDING:
        m_state = secure_state_t::CLOSED;
        goto state_change;
        break;
    case secure_state_t::ENDED:
        m_state = secure_state_t::CLOSED;
        goto state_change;
    case secure_state_t::CLOSING:
        m_state = secure_state_t::CLOSED;
        goto state_change;
    case secure_state_t::CLOSED:
        m_base->close(__cb);
        break;
    }
}


void
gnutls_secure_datagram::send(
    std::string               __message,
    size_t                    __offset,
    size_t                    __length,
    unsigned short            __port,
    std::string               __address,
    event_emitter::callback_t __cb)
{
    udp_address_t key;
    key.address = __address;
    key.port    = __port;
    key.family  = address_family::UNSPECIFIED;
    std::string data = __message.substr(__offset, __length);
    if (data.size() == 0) {
        if (__cb) __cb({});
        return;
    }
    auto connection = _M_find_route(key);
    auto result = gnutls_record_send(
        connection->session.get(), &data[0], data.size());
    if (result < 0)
        _M_emit_error_code(__FUNCTION__, result);
    else { if (__cb) __cb({}); }
}


void
gnutls_secure_datagram::pause()
{ m_base->pause(); }


void
gnutls_secure_datagram::resume()
{ m_base->resume(); }


void
gnutls_secure_datagram::set_broadcast(bool __flag)
{ m_base->set_broadcast(__flag); }


void
gnutls_secure_datagram::set_multicast_interface(std::string __interface)
{ m_base->set_multicast_interface(__interface); }


void
gnutls_secure_datagram::set_multicast_loopback(bool __flag)
{ m_base->set_multicast_loopback(__flag); }


void
gnutls_secure_datagram::set_multicast_ttl(unsigned char __time_to_live)
{ m_base->set_multicast_ttl(__time_to_live); }


void
gnutls_secure_datagram::set_ttl(unsigned char __time_to_live)
{ m_base->set_ttl(__time_to_live); }


void
gnutls_secure_datagram::set_event_observer(
    udp_socket_observer_interface* __fast_events)
{ m_fast_events = __fast_events; }


// ----------------------------------------------------------------------------
// udp_socket_observer_interface
// ----------------------------------------------------------------------------


void
gnutls_secure_datagram::on_close()
{
    if (m_fast_events)
        m_fast_events->on_close();
    else event_emitter::emit("close");
}


void
gnutls_secure_datagram::on_connect()
{
    if (m_fast_events)
        m_fast_events->on_connect();
    else event_emitter::emit("connect");
}


void
gnutls_secure_datagram::on_error(const std::string& __message)
{
    if (m_fast_events)
        m_fast_events->on_error(__message);
    else event_emitter::emit("error", __message);
}


void
gnutls_secure_datagram::on_listening()
{
    if (m_fast_events)
        m_fast_events->on_listening();
    else event_emitter::emit("listening");
}


void
gnutls_secure_datagram::on_message(
    const std::string&   __data,
    const udp_address_t& __address)
{
    V(__FUNCTION__ << "| " << __address.address << ":" << __address.port);

    auto state = 0;
    route_t connection;
    udp_address_t address;
    address.port = __address.port;

    // V(m_base->address().address << " vs " << __address.address);
    // V(m_base->address().port << " vs " << __address.port);
    // V(m_base->address().address.size() << " vs " << __address.address.size());

    if (__address.address == LOOPBACK) {
        V("route to loopback-client\n");
        address.address = m_base->address().address;
    }
    else if (
        __address.address == m_base->address().address &&
        __address.port    == m_base->address().port) {
        V("route to loopback-server\n");
        address.address = LOOPBACK;
    }
    else {
        V("normal route\n");
        address.address = __address.address;
    }

    if (address.port == 0) {
        throw impact_error("");
    }

    V("next route: " << address.address << ":" << address.port);

    BEGIN_LOCK(m_route_mtx)
    auto token = m_routes.find(address);
    if (token != m_routes.end()) {
        state = 1;
        connection = token->second;
    }
    END_LOCK(m_route_mtx)

    if (state == 0 && m_server_enabled) {
        state = 2;
        connection = _M_create_server_agent(__data, __address);
    }

    // invoke events outside the lock so we don't deadlock
    if (connection) {
        switch (state) {
        case 1:
            V("process message for "
                << connection->address.address << ":" << connection->address.port
                << " aka "
                << address.address << ":" << address.port);
            _S_process_message(__data, connection);
            break;
        case 2:
            V("create server agent");
            event_emitter::emit("connection", __address);
            V("trying server-agent handshake");
            _M_try_handshake(connection);
            break;
        default:
            V("drop message");
            /*drop message*/
            break;
        }
    }
}
