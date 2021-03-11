/**
 * Created by TekuConcept on March 7, 2021
 */

#include "sockets/gnutls_secure_server.h"
#include "sockets/gnutls_secure_client.h"
#include "utils/impact_error.h"

using namespace impact;


gnutls_secure_server::gnutls_secure_server(tcp_server_t __base)
: m_base(__base),
  m_x509_credentials(nullptr),
  m_fast_events(nullptr)
{
    if (m_base == nullptr)
        throw impact_error("provided TCP server is not usable");

    _M_init_gnutls_session();

    this->forward(m_base.get());
    m_base->set_event_observer(this);
}


void
gnutls_secure_server::_M_init_gnutls_session()
{
    int result;

    {
        gnutls_certificate_credentials_t credentials;
        result = gnutls_certificate_allocate_credentials(&credentials);
        if (result < 0) goto error;
        m_x509_credentials = std::shared_ptr<gnutls_certificate_credentials_st>(
            credentials,
            [](gnutls_certificate_credentials_st* p)
            { gnutls_certificate_free_credentials(p); }
        );
        if (result < 0) goto error;
        #if GNUTLS_VERSION_NUMBER >= 0x030506
        result = gnutls_certificate_set_known_dh_params(
            m_x509_credentials.get(), GNUTLS_SEC_PARAM_MEDIUM);
        if (result < 0) goto error;
        #endif
    }

    {
        gnutls_priority_st* priority_cache;
        result = gnutls_priority_init(&priority_cache, NULL, NULL);
        if (result < 0) goto error;
        m_priority_cache = std::shared_ptr<gnutls_priority_st>(
            priority_cache,
            [](gnutls_priority_st* p)
            { gnutls_priority_deinit(p); }
        );
    }

    return;

    error:
    throw impact_error(
        std::string(__FUNCTION__) +
        std::string(": ") +
        std::string(gnutls_strerror(result)));
}


void
gnutls_secure_server::_M_emit_error_code(
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
gnutls_secure_server::_M_emit_error_message(std::string __message)
{
    if (m_fast_events)
        m_fast_events->on_error(__message);
    else event_emitter::emit("error", __message);
}


gnutls_x509_crt_fmt_t
gnutls_secure_server::_M_get_gnutls_format(secure_format_t __format)
{
    switch (__format) {
    case secure_format_t::DER: return GNUTLS_X509_FMT_DER;
    case secure_format_t::PEM: return GNUTLS_X509_FMT_PEM;
    default:                   return GNUTLS_X509_FMT_PEM;
    }
}


// ----------------------------------------------------------------------------
// secure_server_interface
// ----------------------------------------------------------------------------


void
gnutls_secure_server::set_x509_trust(
    std::string     __trust,
    secure_format_t __format)
{
    if (__trust.size() == 0) {
        _M_emit_error_message("[security] X509 trust is empty");
        return;
    }
    gnutls_datum_t trust;
    trust.data = (unsigned char*)&__trust[0];
    trust.size = (unsigned int)__trust.size();
    auto result = gnutls_certificate_set_x509_trust_mem(
        m_x509_credentials.get(),
        &trust,
        _M_get_gnutls_format(__format));
    if (result < 0) _M_emit_error_code(__FUNCTION__, result);
}


void
gnutls_secure_server::set_x509_cert_revoke_list(
    std::string     __crl,
    secure_format_t __format)
{
    if (__crl.size() == 0) {
        _M_emit_error_message("[security] X509 crl is empty");
        return;
    }
    gnutls_datum_t crl;
    crl.data = (unsigned char*)&__crl[0];
    crl.size = (unsigned int)__crl.size();
    auto result = gnutls_certificate_set_x509_crl_mem(
        m_x509_credentials.get(),
        &crl,
        _M_get_gnutls_format(__format));
    if (result < 0) _M_emit_error_code(__FUNCTION__, result);
}


void
gnutls_secure_server::set_x509_ocsp_request_file(
    std::string __ocsp_request_file,
    size_t      __index)
{
    if (__ocsp_request_file.size() == 0) {
        _M_emit_error_message(
            "[security] X509 oscp request file name is empty");
        return;
    }
    auto result = gnutls_certificate_set_ocsp_status_request_file(
        m_x509_credentials.get(),
        &__ocsp_request_file[0],
        (unsigned int)__index);
    if (result < 0) _M_emit_error_code(__FUNCTION__, result);
}


void
gnutls_secure_server::set_x509_credentials(
    std::string     __key,
    std::string     __certificate,
    secure_format_t __format)
{
    if (__key.size() == 0 || __certificate.size() == 0) {
        _M_emit_error_message("[security] X509 credentials are empty");
        return;
    }
    gnutls_datum_t key;
    key.data = (unsigned char*)&__key[0];
    key.size = (unsigned int)__key.size();
    gnutls_datum_t certificate;
    certificate.data = (unsigned char*)&__certificate[0];
    certificate.size = (unsigned int)__certificate.size();
    auto result = gnutls_certificate_set_x509_key_mem(
        m_x509_credentials.get(),
        &certificate, &key,
        _M_get_gnutls_format(__format));
    if (result < 0) _M_emit_error_code(__FUNCTION__, result);
}


// ----------------------------------------------------------------------------
// tcp_server_interface
// ----------------------------------------------------------------------------


tcp_address_t
gnutls_secure_server::address() const
{ return m_base->address(); }


bool
gnutls_secure_server::listening() const
{ return m_base->listening(); }


size_t
gnutls_secure_server::max_connections() const
{ return m_base->max_connections(); }


void
gnutls_secure_server::max_connections(size_t __value)
{ m_base->max_connections(__value); }


void
gnutls_secure_server::close(event_emitter::callback_t __cb)
{ m_base->close(__cb); }


void
gnutls_secure_server::listen(
    std::string               __path,
    event_emitter::callback_t __cb)
{ m_base->listen(__path, __cb); }


void
gnutls_secure_server::listen(
    unsigned short            __port,
    std::string               __host,
    event_emitter::callback_t __cb)
{ m_base->listen(__port, __host, __cb); }


void
gnutls_secure_server::set_event_observer(
    tcp_server_observer_interface* __fast_events)
{ m_fast_events = __fast_events; }


// ----------------------------------------------------------------------------
// tcp_server_observer_interface
// ----------------------------------------------------------------------------


void
gnutls_secure_server::on_close()
{
    if (m_fast_events)
        m_fast_events->on_close();
    else event_emitter::emit("close");
}


void
gnutls_secure_server::on_connection(const tcp_client_t& __connection)
{
    if (__connection == nullptr) return;
    secure_client_t secure_connection =
        secure_client_t(new gnutls_secure_client(
            __connection,
            m_x509_credentials,
            m_priority_cache
        ));
    if (m_fast_events)
        m_fast_events->on_connection(secure_connection);
    else event_emitter::emit("connection", secure_connection);
}


void
gnutls_secure_server::on_error(const std::string& __message)
{
    if (m_fast_events)
        m_fast_events->on_error(__message);
    else event_emitter::emit("error", __message);
}


void
gnutls_secure_server::on_listening()
{
    if (m_fast_events)
        m_fast_events->on_listening();
    else event_emitter::emit("listening");
}
