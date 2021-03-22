/**
 * Created by TekuConcept on March 7, 2021
 */

#ifndef IMPACT_INTERFACES_SECURE_SERVER_INTERFACE_H
#define IMPACT_INTERFACES_SECURE_SERVER_INTERFACE_H

#include <string>
#include <memory>
#include "interfaces/tcp_server_interface.h"
#include "interfaces/secure_x509_certificate_interface.h"

namespace impact {

    class secure_server_interface :
        public tcp_server_interface,
        public secure_x509_certificate_interface
    {
    public:
        virtual ~secure_server_interface() = default;
    };
    typedef std::shared_ptr<secure_server_interface> secure_server_t;

} /* namespace impact */

#endif /* IMPACT_INTERFACES_SECURE_SERVER_INTERFACE_H */