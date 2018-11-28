/**
 * Created by TekuConcept on July 7, 2018
 */

#include "sockets/networking.h"

#include <cstring>              // memcpy
#include <sstream>              // ostringstream
#include <iomanip>
#include <map>

#include "utils/environment.h"
#include "utils/impact_error.h"
#include "utils/errno.h"
#include "sockets/generic.h"
#include "sockets/types.h"

#if defined(__OS_LINUX__)
    #include "sockets/basic_socket.h"
#endif

#if defined(__OS_WINDOWS__)
    #include <iphlpapi.h>
    #include <ws2tcpip.h>
    #pragma comment (lib, "IPHLPAPI.lib")
    #pragma comment (lib, "Ws2_32.lib")
#else /* NIX */
    #include <ifaddrs.h>             // getifaddrs(), freeifaddrs()
    #include <sys/ioctl.h>           // ioctl()
    #include <net/if.h>              // ifconf
    #include <arpa/inet.h>           // inet_ntop(), inet_pton
    #include <netinet/in.h>          // INET_ADDRSTRLEN, INET6_ADDRSTRLEN
    #if defined(__OS_APPLE__)
        #include <net/if_types.h>    // IFT_XXX
        #include <net/if_dl.h>       // sockaddr_dl
    #endif /* __APPLE__ */
    #if defined(__OS_LINUX__)
        #include <net/if.h>          // if_indextoname()
        #include <net/if_arp.h>      // ARPHRD_XXX
        #include <linux/netlink.h>   // NLM_F_DUMP, NLM_F_REQUEST, NLM_F_MULTI, ...
        #include <linux/rtnetlink.h> // RTM_GETROUTE
        #include <unistd.h>          // getpid()
    #endif /* __LINUX__ */
#endif /* __WINDOWS__ */


#include <iostream>
#define VERBOSE(x) std::cout << x << std::endl

#if defined(__OS_APPLE__)
    // some apple sources are incomplete
    // net/if_types.h doesn't have every numeric definition
    // for convinience interfaces type values used are listed here

    // IFT_ETHER    0x6
    // IFT_ISO88026 0xa
    // IFT_XETHER   0x1a
    // IFT_PPP      0x17
    // IFT_ATM      0x25
    // IFT_IEEE1394 0x90

    #ifndef IFT_IEEE80211
        #define IFT_IEEE80211 0x47
    #endif

    #ifndef IFT_REACHDSL
        #define IFT_REACHDSL 0xc0
    #endif
#endif

#define CATCH_ASSERT(code)\
    try { code }\
    catch (impact_error e) { throw; }\
    catch (...) { throw impact_error(error_string(imperr::UNKNOWN)); }

#define ASSERT(cond)\
    if (!(cond)) throw impact_error(internal::error_message());

#define IMPACT_INET_ADDRESS_SIZE   4 /*  32 bits */
#define IMPACT_INET6_ADDRESS_SIZE 16 /* 128 bits */

using netinterface   = impact::networking::netinterface;
using interface_type = impact::networking::interface_type;

namespace impact {
namespace internal {
    std::shared_ptr<struct sockaddr> copy_sockaddr_to_ptr(
        const struct sockaddr*);
#if defined(__OS_WINDOWS__)
    void traverse_adapters(std::vector<netinterface>&, PIP_ADAPTER_ADDRESSES);
    void traverse_unicast(std::vector<netinterface>&, netinterface,
        PIP_ADAPTER_UNICAST_ADDRESS);
    void set_ipv4_interface(const struct sockaddr*,unsigned char,netinterface*);
    void set_ipv6_interface(const struct sockaddr*,unsigned char,netinterface*);
    interface_type get_interface_type(unsigned int);
#else /* NIX */
    void traverse_links(const struct ifaddrs*, std::vector<netinterface>*);
    interface_type get_interface_type(unsigned short);
#if defined(__OS_LINUX__)
    void set_interface_type_mac(netinterface*);
    int read_netlink_socket(basic_socket*, char*, unsigned int, unsigned int,
        unsigned int);
#else /* __OS_APPLE__ */
    void set_interface_type_mac(const struct ifaddrs*, netinterface*,
        std::map<std::string,netinterface>*);
#endif
#endif /* __OS_WINDOWS__ */
}}


using namespace impact;


netinterface::netinterface()
: name(""), type(interface_type::OTHER), flags(0),
  ipv4(false), ipv6(false)
{}


std::string
networking::sockaddr_to_string(const struct sockaddr* __address)
{
    if (__address == NULL) return "[no address]";

    std::string result;
    if (__address->sa_family == AF_INET) {
        result.resize(INET_ADDRSTRLEN);
        auto status = inet_ntop(AF_INET,
            &((struct sockaddr_in*)__address)->sin_addr,
            &result[0], sizeof(struct sockaddr_in));
        if (status == NULL) {
            std::ostringstream os;
            struct sockaddr_in& copy = *(struct sockaddr_in*)__address;
        #if defined __OS_WINDOWS__
            os << copy.sin_addr.S_un.S_un_b.s_b1 << ".";
            os << copy.sin_addr.S_un.S_un_b.s_b1 << ".";
            os << copy.sin_addr.S_un.S_un_b.s_b3 << ".";
            os << copy.sin_addr.S_un.S_un_b.s_b4;
        #else
            os << (int)(0xFF & (copy.sin_addr.s_addr >> 24)) << ".";
            os << (int)(0xFF & (copy.sin_addr.s_addr >> 16)) << ".";
            os << (int)(0xFF & (copy.sin_addr.s_addr >>  8)) << ".";
            os << (int)(0xFF & (copy.sin_addr.s_addr      ));
        #endif
            return os.str();
        }
    }
    else if (__address->sa_family == AF_INET6) {
        result.resize(INET6_ADDRSTRLEN);
        auto status = inet_ntop(AF_INET6,
            &((struct sockaddr_in6*)__address)->sin6_addr,
            &result[0], sizeof(struct sockaddr_in6));
        if (status == NULL) {
            std::ostringstream os;
            struct sockaddr_in6& copy = *(struct sockaddr_in6*)__address;
            os << std::hex << std::setfill('0');
        #if defined __OS_WINDOWS__
            for (int i = 0; i < IPV6_ADDRESS_SIZE - 2; i += 2)
                os << std::setw(2) << (int)copy.sin6_addr.u.Byte[i]   <<
                      std::setw(2) << (int)copy.sin6_addr.u.Byte[i+1] << ":";
            os<<std::setw(2)<<(int)copy.sin6_addr.u.Byte[IPV6_ADDRESS_SIZE-2] <<
                std::setw(2)<<(int)copy.sin6_addr.u.Byte[IPV6_ADDRESS_SIZE-1];
        #else
            for (int i = 0; i < IPV6_ADDRESS_SIZE - 2; i += 2)
                os << std::setw(2) << (int)copy.sin6_addr.s6_addr[i] <<
                      std::setw(2) << (int)copy.sin6_addr.s6_addr[i+1] << ":";
            os<<std::setw(2)<<(int)copy.sin6_addr.s6_addr[IPV6_ADDRESS_SIZE-2]<<
                std::setw(2)<<(int)copy.sin6_addr.s6_addr[IPV6_ADDRESS_SIZE-1];
        #endif
            os << std::dec << std::setfill(' ');
            return os.str();
        }
    }
    else return "[unrecognized address family]";

    return result;
}


std::shared_ptr<struct sockaddr>
internal::copy_sockaddr_to_ptr(const struct sockaddr* __address)
{
    if (__address == NULL) return nullptr;

    std::shared_ptr<struct sockaddr> result;
    switch (__address->sa_family) {
    case AF_INET: {
        struct sockaddr_in* address = new struct sockaddr_in;
        memcpy(address, __address, sizeof(struct sockaddr_in));
        result = std::shared_ptr<struct sockaddr>((struct sockaddr*)address);
    } break;
    case AF_INET6: {
        struct sockaddr_in6* address = new struct sockaddr_in6;
        memcpy(address, __address, sizeof(struct sockaddr_in6));
        result = std::shared_ptr<struct sockaddr>((struct sockaddr*)address);
    } break;
    default: return nullptr;
    }

    return result;
}


#if defined(__OS_WINDOWS__)

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
|  WINDOWS FUNCTIONS                                                          |
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

std::vector<netinterface>
networking::find_network_interfaces()
{
    std::vector<netinterface> list;
    const auto MAX_RETRY = 3;
    DWORD status;
    DWORD size = 15000;
    PIP_ADAPTER_ADDRESSES adapter_addresses;
    auto flags = GAA_FLAG_INCLUDE_PREFIX |
                 GAA_FLAG_SKIP_ANYCAST   |
                 GAA_FLAG_SKIP_MULTICAST |
                 GAA_FLAG_SKIP_DNS_SERVER;
    auto retries = 0;

    do {
        adapter_addresses = (PIP_ADAPTER_ADDRESSES)malloc(size);
        if (adapter_addresses == NULL)
            throw impact_error("Memory allocation failed");

        status = GetAdaptersAddresses(
            AF_UNSPEC,
            flags,
            NULL,
            adapter_addresses,
            &size
        );

        if (status == ERROR_BUFFER_OVERFLOW) {
            free(adapter_addresses);
            adapter_addresses = NULL;
            retries++;
        }
        else break;
    } while ((status == ERROR_BUFFER_OVERFLOW) && (retries < MAX_RETRY));

    if (retries >= MAX_RETRY) {
        if (adapter_addresses) {
            free(adapter_addresses);
            adapter_addresses = NULL;
        }
        throw impact_error("Inconsistent buffer size for Adapter Addresses");
    }

    internal::traverse_adapters(list, adapter_addresses);
    free(adapter_addresses);
    return list;
}


void
internal::traverse_adapters(
    std::vector<netinterface>& __list,
    PIP_ADAPTER_ADDRESSES      __adapters)
{
    for (PIP_ADAPTER_ADDRESSES adapter = __adapters;
        adapter != NULL;
        adapter = adapter->Next) {
        netinterface token;
        token.name          = adapter->AdapterName;
        token.friendly_name = to_narrow_string(adapter->FriendlyName);
        token.type          = get_interface_type(adapter->IfType);
        token.flags         = (unsigned int)adapter->Flags;

        if (adapter->PhysicalAddressLength != 0) {
            token.mac.resize(adapter->PhysicalAddressLength);
            std::memcpy(
                &token.mac[0],
                adapter->PhysicalAddress,
                token.mac.size());
        }
        else { // be consistent with linux
            token.mac.resize(6);
            std::memset(&token.mac[0], 0, token.mac.size());
        }

        traverse_unicast(__list, token, adapter->FirstUnicastAddress);
        __list.push_back(token);
    }
}


void
internal::traverse_unicast(
    std::vector<netinterface>&  __list,
    netinterface                __token,
    PIP_ADAPTER_UNICAST_ADDRESS __addresses)
{
    for (PIP_ADAPTER_UNICAST_ADDRESS address = __addresses;
        address != NULL;
        address = address->Next) {

        netinterface token = __token; // clone
        auto socket_address = address->Address.lpSockaddr;

        if (socket_address->sa_family == AF_INET)
            set_ipv4_interface(socket_address,
                address->OnLinkPrefixLength, &token);
        else if (socket_address->sa_family == AF_INET6)
            set_ipv6_interface(socket_address,
                address->OnLinkPrefixLength, &token);
        // don't add link interfaces to interface list
        else if (socket_address->sa_family == AF_LINK) continue;

        __list.push_back(token);
    }
}


void
internal::set_ipv4_interface(
    const struct sockaddr* __socket_address,
    unsigned char          __prefix_length,
    netinterface*          __token)
{
    struct sockaddr_in* addr = new sockaddr_in;
    memcpy(addr, __socket_address, sizeof(struct sockaddr_in));
    __token->address = std::shared_ptr<struct sockaddr>((struct sockaddr*)addr);
    __token->ipv4 = true;
    __token->ipv6 = false;
    struct sockaddr_in* mask = new struct sockaddr_in;
    mask->sin_family = __socket_address->sa_family;
    ConvertLengthToIpv4Mask(
        __prefix_length,
        (PULONG)&mask->sin_addr
    );
    __token->netmask = std::shared_ptr<struct sockaddr>((struct sockaddr*)mask);
    struct sockaddr_in* broadcast = new struct sockaddr_in;
    broadcast->sin_family = __socket_address->sa_family;
    unsigned long A = *(unsigned long*)&mask->sin_addr;
    unsigned long B =
        *(unsigned long*)&((struct sockaddr_in*)__socket_address)->sin_addr;
    unsigned long C = (A&B | ~A);
    broadcast->sin_addr = *(struct in_addr*)&C;
    __token->broadcast =
        std::shared_ptr<struct sockaddr>((struct sockaddr*)broadcast);
}


void
internal::set_ipv6_interface(
    const struct sockaddr* __socket_address,
    unsigned char          __prefix_length,
    netinterface*          __token)
{
    struct sockaddr_in6* addr = new sockaddr_in6;
    memcpy(addr, __socket_address, sizeof(struct sockaddr_in6));
    __token->ipv4 = false;
    __token->ipv6 = true;
    __token->address = std::shared_ptr<struct sockaddr>((struct sockaddr*)addr);

    struct sockaddr_in6* mask = new sockaddr_in6;
    memset(mask, 0, sizeof(struct sockaddr_in6));
    mask->sin6_family = __socket_address->sa_family;
    if (__prefix_length <= 128) {
        int prefix_bytes = __prefix_length / 8;
        int prefix_bits  = __prefix_length % 8;
        for (int i = 0; i < prefix_bytes; i++)
            mask->sin6_addr.u.Byte[i] = 0xFF;
        if (prefix_bytes < IPV6_ADDRESS_SIZE)
            mask->sin6_addr.u.Byte[prefix_bytes] =
                (unsigned char)(0xFF << (8 - prefix_bits));
    }
    else { // invalid prefix, make netmask equal to address
        for (int i = 0; i < IPV6_ADDRESS_SIZE; i++)
            mask->sin6_addr.u.Byte[i] = 0xFF;
    }
    __token->netmask = std::shared_ptr<struct sockaddr>((struct sockaddr*)mask);
    struct sockaddr_in6* broadcast = new struct sockaddr_in6;
    broadcast->sin6_family = __socket_address->sa_family;
    for (int i = 0; i < IPV6_ADDRESS_SIZE; i++) {
        auto A = mask->sin6_addr.u.Byte[i];
        auto B = addr->sin6_addr.u.Byte[i];
        broadcast->sin6_addr.u.Byte[i] = (A & B | ~A);
    }
    __token->broadcast =
        std::shared_ptr<struct sockaddr>((struct sockaddr*)broadcast);
}


interface_type
internal::get_interface_type(unsigned int __code)
{
    switch (__code) {
    case IF_TYPE_ETHERNET_CSMACD: return interface_type::ETHERNET;
    case IF_TYPE_IEEE80211:       return interface_type::WIFI;
    case IF_TYPE_IEEE1394:        return interface_type::FIREWIRE;
    case IF_TYPE_PPP:             return interface_type::PPP;
    case IF_TYPE_ATM:             return interface_type::ATM;
    default:                      return interface_type::OTHER;
    }
}


struct networking::netinterface
networking::find_default_route()
{
    // https://docs.microsoft.com/en-us/windows/desktop/api/iphlpapi/nf-iphlpapi-getipforwardtable
    struct netinterface result;
    return result;
}

#else /* NIX */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
|  LINUX | OSX FUNCTIONS                                                      |
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

std::vector<netinterface>
networking::find_network_interfaces()
{
    std::vector<netinterface> list;
    struct ::ifaddrs* addresses;
    auto status = ::getifaddrs(&addresses);

    ASSERT(status != -1)

    try { internal::traverse_links(addresses, &list); }
    catch (impact_error) {
        freeifaddrs(addresses);
        throw;
    }
    catch (...) {
        freeifaddrs(addresses);
        throw impact_error("Unknown internal error");
    }

    freeifaddrs(addresses);
    return list;
}


void
internal::traverse_links(
    const struct ifaddrs*               __addresses,
    std::vector<netinterface>*          __list)
{
    std::map<std::string,netinterface> hardware;

    // WARNING: ifa_addr may be NULL
    for (auto target = __addresses; target != NULL; target = target->ifa_next) {
        netinterface token;

        token.flags         = target->ifa_flags;
        token.name          = std::string(target->ifa_name);
        token.friendly_name = token.name;

        token.address       = copy_sockaddr_to_ptr(target->ifa_addr);
        token.netmask       = copy_sockaddr_to_ptr(target->ifa_netmask);
        token.broadcast     = copy_sockaddr_to_ptr(target->ifa_broadaddr);
        // TODO: calculate broadcast from netmask and address if broadcast null?

        token.ipv4 = false;
        token.ipv6 = false;
        if (target->ifa_addr != NULL) {
            switch (target->ifa_addr->sa_family) {
            case AF_INET:  token.ipv4 = true; break;
            case AF_INET6: token.ipv6 = true; break;
            }
        }

#if defined(__OS_APPLE__)
        set_interface_type_mac(target, &token, &hardware);
#else
        UNUSED(hardware);
        CATCH_ASSERT(
            set_interface_type_mac(&token);
        )
#endif

        if (target->ifa_addr != NULL) {
            // don't add LINK or PACKET interfaces to list
        #if defined __OS_APPLE__
            if (target->ifa_addr->sa_family != AF_LINK)
        #else /* __OS_LINUX__ */
            if (target->ifa_addr->sa_family != AF_PACKET)
        #endif
                __list->push_back(token);
        }
        else __list->push_back(token);
    }
}


#if defined(__OS_LINUX__)

void
internal::set_interface_type_mac(netinterface* __token)
{
    basic_socket handle;
    CATCH_ASSERT(
        handle = make_socket(
            address_family::INET,
            socket_type::DATAGRAM,
            internet_protocol::DEFAULT
        );
    )

    struct ifreq request;
    std::memcpy(
        request.ifr_name,
        (void*)&__token->name[0],
        __token->name.length() + 1
    );
    auto status = ::ioctl(handle.get(), SIOCGIFHWADDR, &request);

    CATCH_ASSERT(handle.close();)

    if (status == -1) {
        std::ostringstream os;
        os << internal::error_message() << " " << __token->name << std::endl;
        throw impact_error(os.str());
    }

    __token->type = get_interface_type(request.ifr_hwaddr.sa_family);
    __token->mac.resize(6); // standard MAC length (64 bits | 6 bytes)
    std::memcpy(
        &__token->mac[0],
        request.ifr_hwaddr.sa_data,
        __token->mac.size()
    );
}


interface_type
internal::get_interface_type(unsigned short __family)
{
    switch (__family) {
    case ARPHRD_ETHER:
    case ARPHRD_EETHER:             return interface_type::ETHERNET;
    case ARPHRD_IEEE802:
    case ARPHRD_IEEE802_TR:
    case ARPHRD_IEEE80211:
    case ARPHRD_IEEE80211_PRISM:
    case ARPHRD_IEEE80211_RADIOTAP: return interface_type::WIFI;
    case ARPHRD_IEEE1394:           return interface_type::FIREWIRE;
    case ARPHRD_PPP:                return interface_type::PPP;
    case ARPHRD_ATM:                return interface_type::ATM;
    default:                        return interface_type::OTHER;
    }
}


struct networking::netroute
networking::find_default_route()
{
    // Linux: https://stackoverflow.com/questions/15668653/how-to-find-the-default-networking-interface-in-linux
    basic_socket kernel_socket;
    CATCH_ASSERT(
        kernel_socket = make_socket(
            address_family::NETLINK,
            socket_type::DATAGRAM,
            internet_protocol::ROUTE
        );
    )
    
    auto pid = getpid();
    unsigned int message_sequence = 0;
    std::vector<char> message_buffer(8192, '\0');
    struct nlmsghdr* netlink_message_header =
        reinterpret_cast<struct nlmsghdr*>(&message_buffer[0]);
    netlink_message_header->nlmsg_len   = NLMSG_LENGTH(sizeof(struct rtmsg));
    netlink_message_header->nlmsg_type  = RTM_GETROUTE;
    netlink_message_header->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
    netlink_message_header->nlmsg_seq   = message_sequence++;
    netlink_message_header->nlmsg_pid   = pid;

    CATCH_ASSERT(
        kernel_socket.send(
            netlink_message_header,
            netlink_message_header->nlmsg_len
        );
    )

    int length;
    CATCH_ASSERT(
        length = internal::read_netlink_socket(
            &kernel_socket,
            &message_buffer[0],
            message_buffer.size(),
            message_sequence,
            pid
        );
    )
    
    CATCH_ASSERT(kernel_socket.close();)

    struct netroute result;
    for(; NLMSG_OK(netlink_message_header,length);
        netlink_message_header = NLMSG_NEXT(netlink_message_header,length)) {
        
        char ifname[IF_NAMESIZE];
        u_int source_a = 0, gateway_a = 0, destination_a = 0;
        struct rtmsg* route_message = (struct rtmsg*)NLMSG_DATA(netlink_message_header);
    
        if((route_message->rtm_family != AF_INET) ||
            (route_message->rtm_table != RT_TABLE_MAIN)) break;
    
        struct rtattr* route_attribute = (struct rtattr*)RTM_RTA(route_message);
        int route_length = RTM_PAYLOAD(netlink_message_header);
        for (; RTA_OK(route_attribute,route_length);
            route_attribute = RTA_NEXT(route_attribute,route_length)) {
            switch(route_attribute->rta_type) {
            case RTA_OIF: if_indextoname(*(int*)RTA_DATA(route_attribute), ifname); break;
            case RTA_GATEWAY: gateway_a     = *(u_int*)RTA_DATA(route_attribute); break;
            case RTA_PREFSRC: source_a      = *(u_int*)RTA_DATA(route_attribute); break;
            case RTA_DST:     destination_a = *(u_int*)RTA_DATA(route_attribute); break;
            }
        }
        
        if (destination_a == 0) {
            auto source                  = new struct sockaddr_in;
            auto gateway                 = new struct sockaddr_in;
            auto destination             = new struct sockaddr_in;
            source->sin_family           = route_message->rtm_family;
            gateway->sin_family          = route_message->rtm_family;
            destination->sin_family      = route_message->rtm_family;
            source->sin_port             = 0;
            gateway->sin_port            = 0;
            destination->sin_port        = 0;
            source->sin_addr.s_addr      = source_a;
            gateway->sin_addr.s_addr     = gateway_a;
            destination->sin_addr.s_addr = destination_a;
            result.name                  = std::string(ifname);
            result.source                = std::shared_ptr<struct sockaddr>((struct sockaddr*)source);
            result.gateway               = std::shared_ptr<struct sockaddr>((struct sockaddr*)gateway);
            result.destination           = std::shared_ptr<struct sockaddr>((struct sockaddr*)destination);
            break;
        }
    }

    return result;
}


int
internal::read_netlink_socket(
    basic_socket* __kernel_socket,
    char*         __buffer,
    unsigned int  __buffer_length,
    unsigned int  __sequence,
    unsigned int  __pid)
{
    struct nlmsghdr* netlink_message_header;
    int received = 0, message_length = 0;
    do {
        CATCH_ASSERT(
            received = __kernel_socket->recv(
                __buffer,
                __buffer_length - message_length
            );
        )
        netlink_message_header = (struct nlmsghdr*)__buffer;
        if (NLMSG_OK(netlink_message_header, received) == 0)
            throw impact_error("Netlink Error: Corrupt Message");
        else if ((netlink_message_header->nlmsg_type) == NLMSG_ERROR) {
            struct nlmsgerr* netlink_message_error =
                reinterpret_cast<struct nlmsgerr*>(
                    NLMSG_DATA(netlink_message_header));
            throw impact_error(std::string("Netlink Error: ") +
                std::to_string(netlink_message_error->error));
        }
        
        if (netlink_message_header->nlmsg_type == NLMSG_DONE) break;
        else {
            __buffer       += received;
            message_length += received;
            if ((netlink_message_header->nlmsg_flags & NLM_F_MULTI) == 0) break;
        }
    }
    while((netlink_message_header->nlmsg_seq != __sequence) ||
          (netlink_message_header->nlmsg_pid != __pid));
    return message_length;
}

#endif /* __OS_LINUX__ */


#if defined(__OS_APPLE__)

void
internal::set_interface_type_mac(
    const struct ifaddrs*               __target,
    netinterface*                       __token,
    std::map<std::string,netinterface>* __hardware)
{
    // be consistent with linux
    __token->type = interface_type::OTHER;
    __token->mac.resize(6);
    std::memset(&(__token->mac)[0], 0, __token->mac.size());

    if (__target->ifa_addr != NULL) {
        if (__target->ifa_addr->sa_family == AF_LINK) {
            struct sockaddr_dl* sdl = (struct sockaddr_dl*)__target->ifa_addr;
            __token->type           = get_interface_type(sdl->sdl_type);

            if (sdl->sdl_alen != 0) {
                __token->mac.resize(sdl->sdl_alen);
                std::memcpy(
                    &(__token->mac)[0],
                    LLADDR(sdl),
                    __token->mac.size()
                );
            }

            (*__hardware)[__token->name] = *__token;
        }
        else goto iface_history;
    }
    else goto iface_history;

    return;

    iface_history: {
        // WARNING:
        // - this assumes a link token comes first
        // - there is the possibility of duplicate
        // link tokens under the same name but with
        // different mac addresses.
        auto iface = __hardware->find(__token->name);
        if (iface != __hardware->end()) {
            __token->mac   = iface->second.mac;
            __token->type  = iface->second.type;
        }
    }
}


interface_type
internal::get_interface_type(unsigned short __family)
{
    switch (__family) {
    case IFT_ETHER:
    case IFT_XETHER:    return interface_type::ETHERNET;
    case IFT_IEEE80211:
    // DSL/modem router - tested against wifi network,
    // but may also be used for ethernet network
    case IFT_REACHDSL:
    // this is actually a Metropolitan Area Network (MAN)
    // but it can be used for WiFi interfaces
    case IFT_ISO88026:    return interface_type::WIFI;
    case IFT_IEEE1394:  return interface_type::FIREWIRE;
    case IFT_PPP:       return interface_type::PPP;
    case IFT_ATM:       return interface_type::ATM;
    default:            return interface_type::OTHER;
    }
}


struct networking::netinterface
networking::find_default_route()
{
    // OSX:   https://www.freebsd.org/cgi/man.cgi?query=route&apropos=0&sektion=4&manpath=FreeBSD%208.2-RELEASE&arch=default&format=html
    //        https://stackoverflow.com/questions/7639451/reading-the-route-table-on-freebsd
    struct netinterface result;
    return result;
}

#endif /* __OS_APPLE__ */

#endif /* __OS_WINDOWS__ */
