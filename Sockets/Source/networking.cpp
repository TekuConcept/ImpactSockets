/**
 * Created by TekuConcept on July 7, 2018
 */

#include "sockets/networking.h"

#include <sstream>              // ostringstream
#include <cstring>              // memcpy

#include "sockets/environment.h"
#include "sockets/generic.h"
#include "sockets/types.h"
#include "sockets/impact_error.h"

#if defined(__LINUX__)
	#include "sockets/basic_socket.h"
#endif

#if defined(__WINDOWS__)
	#include <winsock2.h>
	#include <iphlpapi.h>
	#pragma comment (lib, "IPHLPAPI.lib")
#else /* NIX */
	#include <ifaddrs.h>          // getifaddrs(), freeifaddrs()
	#include <sys/ioctl.h>        // ioctl()
	#include <net/if.h>           // ifconf
	#if defined(__APPLE__)
		#include <net/if_types.h> // IFT_XXX
		#include <net/if_dl.h>    // sockaddr_dl
	#endif /* __APPLE__ */
	#if defined(__LINUX__)
		#include <net/if_arp.h>   // ARPHRD_XXX
	#endif /* __LINUX__ */
#endif /* __WINDOWS__ */

#if defined(__APPLE__)
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
	catch (...) { throw impact_error("Unknown internal error"); }

#define ASSERT(cond)\
 	if (!(cond)) throw impact_error(internal::error_message());

using netinterface   = impact::networking::netinterface;
using interface_type = impact::networking::interface_type;


namespace impact {
namespace internal {
#if defined(__WINDOWS__)
	void traverse_adapters(std::vector<netinterface>&, PIP_ADAPTER_ADDRESSES);
	void traverse_unicast(std::vector<netinterface>&, netinterface,
    PIP_ADAPTER_UNICAST_ADDRESS);
	interface_type get_interface_type(unsigned int);
#else /* NIX */
	void traverse_links(std::vector<netinterface>&, struct ifaddrs*);
	interface_type get_interface_type(unsigned short);
#if defined(__LINUX__)
	void set_interface_type_mac(netinterface&);
#endif /* __LINUX__ */
#endif /* __WINDOWS__ */
}}


using namespace impact;


netinterface::netinterface()
: flags(0), name(""), address(""), netmask(""),
  broadcast(""), type(interface_type::OTHER), ipv4(false)
{}


#if defined(__WINDOWS__)

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
		token.name  = to_narrow_string(adapter->FriendlyName);
		token.flags = (unsigned int)adapter->Flags;
		token.type  = get_interface_type(adapter->IfType);

		if (adapter->PhysicalAddressLength != 0) {
			token.mac.resize(adapter->PhysicalAddressLength);
			std::memcpy(&token.mac[0], adapter->PhysicalAddress, token.mac.size());
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

		auto socket_address = address->Address.lpSockaddr;
		__token.address = sock_addr_string(socket_address);

		if (socket_address->sa_family == AF_INET) {
			__token.ipv4 = true;

			struct sockaddr_in mask;
			mask.sin_family = socket_address->sa_family;
			ConvertLengthToIpv4Mask(
				address->OnLinkPrefixLength,
				(PULONG)&mask.sin_addr
			);

			__token.netmask      = sock_addr_string((struct sockaddr*)&mask);
			struct sockaddr_in broadcast;
			broadcast.sin_family = socket_address->sa_family;
			unsigned long A      = *(unsigned long*)&mask.sin_addr;
			unsigned long B      =
				*(unsigned long*)&((struct sockaddr_in*)socket_address)->sin_addr;
			unsigned long C      = (A&B | ~A);
			broadcast.sin_addr   = *(struct in_addr*)&C;
			__token.broadcast    = sock_addr_string((struct sockaddr*)&broadcast);
		}
		// TODO: IPv6 mask and broadcast

		__list.push_back(__token);
	}
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

	try { internal::traverse_links(list, addresses); }
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
	std::vector<netinterface>& __list,
	struct ifaddrs*            __addresses)
{
	// WARNING: ifa_addr may be NULL
	for (auto target = __addresses; target != NULL; target = target->ifa_next) {
		netinterface token;

		token.flags      = target->ifa_flags;
		token.name       = std::string(target->ifa_name);
		token.address    = sock_addr_string(target->ifa_addr);
		token.netmask    = sock_addr_string(target->ifa_netmask);
		token.broadcast  = sock_addr_string(target->ifa_broadaddr);
		token.ipv4       = (target->ifa_addr != NULL) ?
			(target->ifa_addr->sa_family == AF_INET) : false;

#if defined(__APPLE__)
		// be consistent with linux
		token.mac.resize(6);
		std::memset(&token.mac[0], 0, token.mac.size());
		
		if (target->ifa_addr != NULL) {
			struct sockaddr_dl* sdl = (struct sockaddr_dl*)target->ifa_addr;
			token.type              = get_interface_type(sdl->sdl_type);

			if (sdl->sdl_alen != 0) {
				token.mac.resize(sdl->sdl_alen);
				memcpy(&token.mac[0], LLADDR(sdl), token.mac.size());
			}
		}
		else token.type = interface_type::OTHER;
#else
		CATCH_ASSERT(
			set_interface_type_mac(token);
		)
#endif

		__list.push_back(token);
	}
}


#if defined(__LINUX__)

void
internal::set_interface_type_mac(netinterface& __token)
{
	basic_socket handle;
	CATCH_ASSERT(
		handle = make_socket(
			socket_domain::INET,
			socket_type::DATAGRAM,
			socket_protocol::DEFAULT
		);
	)

	struct ifreq request;
	std::memcpy(
		request.ifr_name,
		(void*)&__token.name[0],
		__token.name.length() + 1
	);
	auto status = ::ioctl(handle.get(), SIOCGIFHWADDR, &request);

	CATCH_ASSERT(handle.close();)

	if (status == -1) {
		std::ostringstream os;
		os << internal::error_message() << " " << __token.name << std::endl;
		throw impact_error(os.str());
	}

	__token.type = get_interface_type(request.ifr_hwaddr.sa_family);
	__token.mac.resize(6); // standard MAC length (64 bits | 6 bytes)
	std::memcpy(&__token.mac[0],request.ifr_hwaddr.sa_data,__token.mac.size());
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

#endif /* __LINUX__ */


#if defined(__APPLE__)

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
	case IFT_ISO88026:	return interface_type::WIFI;
	case IFT_IEEE1394:  return interface_type::FIREWIRE;
	case IFT_PPP:       return interface_type::PPP;
	case IFT_ATM:       return interface_type::ATM;
	default:            return interface_type::OTHER;
	}
}

#endif /* __APPLE__ */

#endif /* __WINDOWS__ */
