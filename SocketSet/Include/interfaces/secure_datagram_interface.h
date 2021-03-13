/**
 * Created by TekuConcept on March 7, 2021
 */

#ifndef IMPACT_INTERFACES_SECURE_DATAGRAM_INTERFACE_H
#define IMPACT_INTERFACES_SECURE_DATAGRAM_INTERFACE_H

#include "interfaces/udp_socket_interface.h"
#include "interfaces/secure_x509_certificate_interface.h"

namespace impact {

    class secure_datagram_interface :
        public udp_socket_interface,
        public secure_x509_certificate_interface
    {
    public:
        virtual ~secure_datagram_interface() = default;

        virtual const std::string& server_name() const = 0;
        virtual void server_name(std::string host) = 0;
        virtual bool cert_verify_enabled() const = 0;
        virtual void cert_verify_enabled(bool enabled) = 0;
    };

} /* namespace impact */

#endif /* IMPACT_INTERFACES_SECURE_DATAGRAM_INTERFACE_H */
