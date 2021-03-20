/**
 * Created by TekuConcept on March 7, 2021
 */

#ifndef IMPACT_INTERFACES_SECURE_DATAGRAM_INTERFACE_H
#define IMPACT_INTERFACES_SECURE_DATAGRAM_INTERFACE_H

#include <string>
#include <memory>
#include "interfaces/udp_socket_interface.h"
#include "interfaces/secure_x509_certificate_interface.h"

namespace impact {

    class secure_datagram_interface :
        public udp_socket_interface,
        public secure_x509_certificate_interface
    {
    public:
        virtual ~secure_datagram_interface() = default;

        virtual size_t mtu() const = 0;
        virtual void mtu(size_t value) = 0;
        virtual void create(udp_address_t address) = 0;
        virtual void begin(udp_address_t address) = 0;
        virtual void end(udp_address_t address) = 0;
        virtual void destroy(udp_address_t address) = 0;
        virtual const std::string& server_name(udp_address_t address) = 0;
        virtual void server_name(udp_address_t address, std::string host) = 0;
        virtual bool cert_verify_enabled(udp_address_t address) = 0;
        virtual void cert_verify_enabled(udp_address_t address, bool enabled) = 0;

        virtual void set_x509_client_credentials(
            udp_address_t address,
            std::string key,
            std::string certificate,
            secure_format_t format = secure_format_t::PEM) = 0;

        virtual void enable_server(bool enabled) = 0;
    };
    typedef std::shared_ptr<secure_datagram_interface> secure_datagram_t;

} /* namespace impact */

#endif /* IMPACT_INTERFACES_SECURE_DATAGRAM_INTERFACE_H */
