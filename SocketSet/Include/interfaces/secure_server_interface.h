/**
 * Created by TekuConcept on March 7, 2021
 */

#ifndef IMPACT_INTERFACES_SECURE_SERVER_INTERFACE_H
#define IMPACT_INTERFACES_SECURE_SERVER_INTERFACE_H

#include <string>
#include "interfaces/tcp_server_interface.h"
#include "interfaces/secure_types.h"

namespace impact {

    class secure_server_interface : public tcp_server_interface {
    public:
        virtual ~secure_server_interface() = default;

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

#endif /* IMPACT_INTERFACES_SECURE_SERVER_INTERFACE_H */
