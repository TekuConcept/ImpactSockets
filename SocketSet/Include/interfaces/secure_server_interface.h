/**
 * Created by TekuConcept on March 7, 2021
 */

#ifndef IMPACT_INTERFACES_SECURE_SERVER_INTERFACE_H
#define IMPACT_INTERFACES_SECURE_SERVER_INTERFACE_H

#include "interfaces/tcp_server_interface.h"

namespace impact {

    class secure_server_interface : public tcp_server_interface {
    public:
        virtual ~secure_server_interface() = default;
    };

} /* namespace impact */

#endif /* IMPACT_INTERFACES_SECURE_SERVER_INTERFACE_H */
