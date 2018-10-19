/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef _IMPACT_NETWORKING_H_
#define _IMPACT_NETWORKING_H_

#include <string>
#include <vector>

#include "utils/environment.h"
#include "sockets/types.h"

#if defined(__OS_WINDOWS__)
    #include <winsock2.h> /* sockaddr */
#else
    #include <netinet/in.h> /* sockaddr */
#endif

namespace impact {
namespace networking {
    typedef enum class interface_type {
        // !- CROSS-PLATFORM TYPES    -!
        OTHER,
        ETHERNET,
        WIFI,
        FIREWIRE,
        PPP,
        ATM
    } InterfaceType;


    typedef struct netinterface {
        std::string                      name;
        interface_type                   type;
        unsigned int                     flags;
        std::shared_ptr<struct sockaddr> address;
        std::shared_ptr<struct sockaddr> netmask;
        std::shared_ptr<struct sockaddr> broadcast;
        std::vector<unsigned char>       mac;
        bool                             ipv4;
        bool                             ipv6;
        netinterface();
    } NetworkInterface;


    std::string byte_address_to_string(
        std::vector<unsigned char> address)
        /* throw(impact_error) */;
    std::vector<unsigned char> string_to_byte_address(
        socket_domain domain, std::string address)
        /* throw(impact_error) */;
    std::string sockaddr_to_string(const struct sockaddr* address);


    std::vector<struct netinterface> find_network_interfaces()
        /* throw(impact_error) */;
}}

#undef MAC_ADDRESS_LENGTH
#endif
