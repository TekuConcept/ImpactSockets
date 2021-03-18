/**
 * Created by TekuConcept on March 12, 2021
 */

#ifndef IMPACT_INTERFACES_SECURE_X509_CERTIFICATE_INTERFACE_H
#define IMPACT_INTERFACES_SECURE_X509_CERTIFICATE_INTERFACE_H

#include <string>

namespace impact {

    enum class secure_format_t { PEM, DER };
    enum class secure_connection_type_t { CLIENT, SERVER };
    enum class secure_state_t {
        OPENING, OPEN,
        ENDING,  ENDED,
        CLOSING, CLOSED
    };

    class secure_x509_certificate_interface {
    public:
        virtual ~secure_x509_certificate_interface() = default;

        virtual void set_x509_trust(
            std::string trust,
            secure_format_t format = secure_format_t::PEM) = 0;
        virtual void set_x509_cert_revoke_list(
            std::string crl,
            secure_format_t format = secure_format_t::PEM) = 0;
        // ocsp: online certificate status protocol
        virtual void set_x509_ocsp_request_file(
            std::string ocsp_request_file,
            size_t index = 0) = 0;
        virtual void set_x509_credentials(
            std::string key,
            std::string certificate,
            secure_format_t format = secure_format_t::PEM) = 0;
    };

} /* namespace impact */

#endif /* IMPACT_INTERFACES_SECURE_X509_CERTIFICATE_INTERFACE_H */
