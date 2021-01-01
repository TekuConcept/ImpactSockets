/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef IMPACT_NETWORKING_H
#define IMPACT_NETWORKING_H

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
        std::string                      friendly_name;
        interface_type                   type;
		unsigned int                     iface_index;
        unsigned int                     flags;
        std::shared_ptr<struct sockaddr> address;
        std::shared_ptr<struct sockaddr> netmask;
        std::shared_ptr<struct sockaddr> broadcast;
        std::vector<unsigned char>       mac;
        bool                             ipv4;
        bool                             ipv6;
        netinterface();
    } NetworkInterface;
    
    
    typedef struct netroute {
        std::string name;
		unsigned int iface_index;
        std::shared_ptr<struct sockaddr> gateway;
    } NetRoute;


    std::string sockaddr_to_string(const struct sockaddr* address);


    std::vector<struct netinterface> find_network_interfaces()
        /* throw(impact_error) */;
    struct netroute find_default_route()
        /* throw(impact_error) */;
}}

#undef MAC_ADDRESS_LENGTH
#endif
