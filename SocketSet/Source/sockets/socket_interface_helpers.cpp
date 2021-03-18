/**
 * Created by TekuConcept on March 15, 2021
 */

#include <functional>
#include "interfaces/tcp_client_interface.h"
#include "interfaces/udp_socket_interface.h"

namespace impact {

    bool
    operator<(
        const tcp_address_t& lhs,
        const tcp_address_t& rhs)
    {
        int equal = lhs.address.compare(rhs.address);
        return (equal < 0) ||
            (bool)((int)(equal == 0) & (int)(lhs.port < rhs.port));
    }


    bool
    operator<(
        const udp_address_t& lhs,
        const udp_address_t& rhs)
    {
        int equal = lhs.address.compare(rhs.address);
        return (equal < 0) ||
            (bool)((int)(equal == 0) & (int)(lhs.port < rhs.port));
    }

}
