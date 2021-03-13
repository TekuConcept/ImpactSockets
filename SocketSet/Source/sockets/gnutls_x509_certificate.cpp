/**
 * Created by TekuConcept on March 12, 2021
 */

#include "sockets/gnutls_x509_certificate.h"
#include "utils/impact_error.h"

using namespace impact;


gnutls_x509_certificate::gnutls_x509_certificate(
    credentials_t __credentials,
    observer_t    __error_observer)
: m_credentials(__credentials)
{
    int result = 0;

    set_error_observer(__error_observer);

    if (__credentials == nullptr) {
        gnutls_certificate_credentials_t credentials;
        result = gnutls_certificate_allocate_credentials(&credentials);
        if (result < 0) goto error;
        m_credentials = std::shared_ptr<gnutls_certificate_credentials_st>(
            credentials,
            [](gnutls_certificate_credentials_st* p)
            { gnutls_certificate_free_credentials(p); }
        );
        /* Sets the system trusted CAs for Internet PKI */
        result = gnutls_certificate_set_x509_system_trust(
            m_credentials.get());
        if (result < 0) goto error;
    }

    return;

    error:
    m_error_callback(
        std::string(__FUNCTION__) +
        std::string(": ") +
        std::string(gnutls_strerror(result)));
}


void
gnutls_x509_certificate::set_x509_trust(
    std::string     __trust,
    secure_format_t __format)
{
    if (__trust.size() == 0) {
        m_error_callback("[security] X509 trust is empty");
        return;
    }
    gnutls_datum_t trust;
    trust.data = (unsigned char*)&__trust[0];
    trust.size = (unsigned int)__trust.size();
    auto result = gnutls_certificate_set_x509_trust_mem(
        m_credentials.get(), &trust, _M_get_gnutls_format(__format));
    if (result < 0)
        m_error_callback(gnutls_strerror(result));
}


void
gnutls_x509_certificate::set_x509_cert_revoke_list(
    std::string     __crl,
    secure_format_t __format)
{
    if (__crl.size() == 0) {
        m_error_callback("[security] X509 crl is empty");
        return;
    }
    gnutls_datum_t crl;
    crl.data = (unsigned char*)&__crl[0];
    crl.size = (unsigned int)__crl.size();
    auto result = gnutls_certificate_set_x509_crl_mem(
        m_credentials.get(), &crl, _M_get_gnutls_format(__format));
    if (result < 0) m_error_callback(gnutls_strerror(result));
}


void
gnutls_x509_certificate::set_x509_ocsp_request_file(
    std::string __ocsp_request_file,
    size_t      __index)
{
    if (__ocsp_request_file.size() == 0) {
        m_error_callback(
            "[security] X509 oscp request file name is empty");
        return;
    }
    auto result = gnutls_certificate_set_ocsp_status_request_file(
        m_credentials.get(),
        &__ocsp_request_file[0],
        (unsigned int)__index);
    if (result < 0) m_error_callback(gnutls_strerror(result));
}


void
gnutls_x509_certificate::set_x509_credentials(
    std::string     __key,
    std::string     __certificate,
    secure_format_t __format)
{
    if (__key.size() == 0 || __certificate.size() == 0) {
        m_error_callback("[security] X509 credentials are empty");
        return;
    }
    gnutls_datum_t key;
    key.data = (unsigned char*)&__key[0];
    key.size = (unsigned int)__key.size();
    gnutls_datum_t certificate;
    certificate.data = (unsigned char*)&__certificate[0];
    certificate.size = (unsigned int)__certificate.size();
    auto result = gnutls_certificate_set_x509_key_mem(
        m_credentials.get(),
        &certificate, &key,
        _M_get_gnutls_format(__format));
    if (result < 0) m_error_callback(gnutls_strerror(result));
}


void
gnutls_x509_certificate::set_error_observer(observer_t __cb)
{
    if (__cb != nullptr) m_error_callback = __cb;
    else m_error_callback = std::bind(
        &gnutls_x509_certificate::_M_default_observer_callback,
        this, std::placeholders::_1);
}


gnutls_x509_crt_fmt_t
gnutls_x509_certificate::_M_get_gnutls_format(secure_format_t __format)
{
    switch (__format) {
    case secure_format_t::DER: return GNUTLS_X509_FMT_DER;
    case secure_format_t::PEM: return GNUTLS_X509_FMT_PEM;
    default:                   return GNUTLS_X509_FMT_PEM;
    }
}


void
gnutls_x509_certificate::_M_default_observer_callback(std::string __message)
{ throw impact_error(__message); }
