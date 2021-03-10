/**
 * Created by TekuConcept on March 7, 2021
 */

#ifndef IMPACT_INTERFACES_SECURE_DATAGRAM_INTERFACE_H
#define IMPACT_INTERFACES_SECURE_DATAGRAM_INTERFACE_H

#include "interfaces/udp_socket_interface.h"

namespace impact {

    class secure_datagram_interface : public udp_socket_interface {
    public:
        virtual ~secure_datagram_interface() = default;
    };

} /* namespace impact */

#endif /* IMPACT_INTERFACES_SECURE_DATAGRAM_INTERFACE_H */
