/**
 * Created by TekuConcept on March 12, 2021
 */

#ifndef IMPACT_SOCKETS_GNUTLS_X509_CERTIFICATE_H
#define IMPACT_SOCKETS_GNUTLS_X509_CERTIFICATE_H

#include <memory>
#include <functional>
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>
#include "interfaces/secure_x509_certificate_interface.h"

namespace impact {

    class gnutls_x509_certificate : public secure_x509_certificate_interface {
    public:
        typedef std::shared_ptr<gnutls_certificate_credentials_st>
            credentials_t;
        typedef std::function<void(std::string)> observer_t;

        gnutls_x509_certificate(
            credentials_t credentials = nullptr,
            observer_t error_observer = nullptr);

        void set_x509_trust(
            std::string trust,
            secure_format_t format = secure_format_t::PEM) override;
        void set_x509_cert_revoke_list(
            std::string crl,
            secure_format_t format = secure_format_t::PEM) override;
        // ocsp: online certificate status protocol
        void set_x509_ocsp_request_file(
            std::string ocsp_request_file,
            size_t index = 0) override;
        void set_x509_credentials(
            std::string key,
            std::string certificate,
            secure_format_t format = secure_format_t::PEM) override;

        inline gnutls_certificate_credentials_st* get()
        { return m_credentials.get(); }
        void set_error_observer(observer_t cb);

    private:
        credentials_t m_credentials;
        observer_t    m_error_callback;

        gnutls_x509_crt_fmt_t _M_get_gnutls_format(secure_format_t);
        void _M_default_observer_callback(std::string);
    };

} /* namespace impact */

#endif /* IMPACT_SOCKETS_GNUTLS_X509_CERTIFICATE_H */
