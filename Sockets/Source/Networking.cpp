/**
 * Created by TekuConcept on July 7, 2018
 */

#include "Environment.h"
#include "Generic.h"
#include "Networking.h"
#include "SocketTypes.h"
#include "SocketInterface.h"
#include <stdexcept>            // runtime_error
#include <sstream>              // ostringstream

#if defined(__WINDOWS__)
    #include <winsock2.h>
#else /* NIX */
    #include <ifaddrs.h>        // getifaddrs(), freeifaddrs()
    #include <sys/ioctl.h>      // ioctl()
    #include <net/if.h>         // ifconf
#endif /* __WINDOWS__ */

#if defined(__APPLE__)
	#include <net/if_types.h>   // IFT_XXX
	#include <net/if_dl.h>      // sockaddr_dl
#endif /* __APPLE__ */

#if defined(__LINUX__)
    #include <net/if_arp.h>     // ARPHRD_XXX
#endif /* __LINUX__ */

#define CATCH_ASSERT(title,code)\
 	try { code }\
 	catch (std::runtime_error e) {\
 		std::string message( title );\
 		message.append(e.what());\
 		throw std::runtime_error(message);\
 	}

#define ASSERT(title,cond)\
 	if (cond) {\
 		std::string message( title );\
 		message.append(Internal::getErrorMessage());\
 		throw std::runtime_error(message);\
 	}

using NetInterface  = Impact::Networking::NetInterface;
using InterfaceType = Impact::Networking::InterfaceType;
    
namespace Impact {
namespace Internal {
    std::vector<NetInterface> getNetworkInterfaces_Win();
	void gniWinAdapterTraverse(std::vector<NetInterface>&, void*);
	void gniWinUnicastTraverse(std::vector<NetInterface>&, NetInterface, void*);
	InterfaceType gniWinGetInterfaceType(unsigned int);
	std::vector<NetInterface> getNetworkInterfaces_Nix();
	void gniNixLinkTraverse(std::vector<NetInterface>&, struct ifaddrs*);
	InterfaceType gniLinuxGetInterfaceType(SocketDomain, const std::string&);
	InterfaceType gniLinuxGetInterfaceType(unsigned short);
	InterfaceType gniOSXGetInterfaceType(unsigned short);
}}


using namespace Impact;


Networking::NetInterface::NetInterface() :
	flags(0), name(""), address(""), netmask(""),
	broadcast(""), type(InterfaceType::OTHER), ipv4(false) {}


std::vector<NetInterface> Networking::findNetworkInterfaces() {
#if defined(_MSC_VER)
	CATCH_ASSERT(
		"SocketInterface::getNetworkInterfaces()\n",
		return Internal::getNetworkInterfaces_Win();
	);
#else /* OSX|LINUX */
	CATCH_ASSERT(
		"SocketInterface::getNetworkInterfaces()\n",
		return Internal::getNetworkInterfaces_Nix();
	);
#endif
}


std::vector<NetInterface> Internal::getNetworkInterfaces_Win() {
	std::vector<NetInterface> list;
#if defined(_MSC_VER)
	const auto MAX_RETRY = 3;
	DWORD status;
	DWORD size = 15000;
	PIP_ADAPTER_ADDRESSES adapterAddresses;
	auto flags = GAA_FLAG_INCLUDE_PREFIX |
				 GAA_FLAG_SKIP_ANYCAST   |
				 GAA_FLAG_SKIP_MULTICAST |
				 GAA_FLAG_SKIP_DNS_SERVER;

	auto retries = 0;
	do {
		adapterAddresses = (PIP_ADAPTER_ADDRESSES)malloc(size);
		if (adapterAddresses == NULL) {
			throw std::runtime_error(
				"SocketInterface::getNetworkInterfaces_Win(1)\n"
				"Memory allocation failed.");
		}
		status = GetAdaptersAddresses(AF_UNSPEC, flags, NULL, adapterAddresses, &size);
		if (status == ERROR_BUFFER_OVERFLOW) {
			free(adapterAddresses);
			adapterAddresses = NULL;
			retries++;
		}
		else break;
	} while ((status == ERROR_BUFFER_OVERFLOW) && (retries < MAX_RETRY));

	if (retries >= MAX_RETRY) {
		throw std::runtime_error(
			"SocketInterface::getNetworkInterfaces_Win(2)\n"
			"Failed to identify buffer size for Adapter Addresses.");
	}

	gniWinAdapterTraverse(list, (void*)adapterAddresses);
	free(adapterAddresses);
#endif
	return list;
}


void Internal::gniWinAdapterTraverse(
	std::vector<NetInterface>& list, void* adapters) {
#if defined(_MSC_VER)
	for (PIP_ADAPTER_ADDRESSES adapter = (PIP_ADAPTER_ADDRESSES)adapters;
		adapter != NULL; adapter = adapter->Next) {
		NetInterface token;
		token.name  = Internal::toNarrowString(adapter->FriendlyName);
		token.flags = (unsigned int)adapter->Flags;
		token.type = gniWinGetInterfaceType(adapter->IfType);
		gniWinUnicastTraverse(list, token, adapter->FirstUnicastAddress);
		list.push_back(token);
	}
#else
	UNUSED(list);
	UNUSED(adapters);
#endif
}


void Internal::gniWinUnicastTraverse(std::vector<NetInterface>& list,
	NetInterface token, void* addresses) {
#if defined(_MSC_VER)
	for (PIP_ADAPTER_UNICAST_ADDRESS address =
		(PIP_ADAPTER_UNICAST_ADDRESS)addresses;
		address != NULL; address = address->Next) {

		auto socketAddress = address->Address.lpSockaddr;
		token.address = sockAddr2String(socketAddress);

		if (socketAddress->sa_family == AF_INET) {
			token.ipv4 = true;

			struct sockaddr_in mask;
			mask.sin_family = socketAddress->sa_family;
			ConvertLengthToIpv4Mask(
				address->OnLinkPrefixLength,
				(PULONG)&mask.sin_addr
			);
			token.netmask = sockAddr2String((struct sockaddr*)&mask);

			struct sockaddr_in broadcast;
			broadcast.sin_family = socketAddress->sa_family;
			unsigned long A = *(unsigned long*)&mask.sin_addr;
			unsigned long B = *(unsigned long*)&(
				(struct sockaddr_in*)socketAddress)->sin_addr;
			unsigned long C = (A&B | ~A);
			broadcast.sin_addr = *(struct in_addr*)&C;
			token.broadcast = sockAddr2String((struct sockaddr*)&broadcast);
		}
		// TODO: IPv6 mask and broadcast

		list.push_back(token);
	}
#else
	UNUSED(list);
	UNUSED(token);
	UNUSED(addresses);
#endif
}


InterfaceType Internal::gniWinGetInterfaceType(unsigned int code) {
#if defined(_MSC_VER)
	switch (code) {
	case IF_TYPE_ETHERNET_CSMACD:	return InterfaceType::ETHERNET;
	case IF_TYPE_IEEE80211:			return InterfaceType::WIFI;
	case IF_TYPE_IEEE1394:			return InterfaceType::FIREWIRE;
	case IF_TYPE_PPP:				return InterfaceType::PPP;
	case IF_TYPE_ATM:       return InterfaceType::ATM;
	default:						return InterfaceType::OTHER;
	}
#else
	UNUSED(code);
	return InterfaceType::OTHER;
#endif
}


std::vector<NetInterface> Internal::getNetworkInterfaces_Nix() {
	std::vector<NetInterface> list;
#ifndef _MSC_VER
	struct ::ifaddrs* addresses;
	auto status = ::getifaddrs(&addresses);

	ASSERT("SocketInterface::getNetworkInterfaces_Nix(1)\n", status == -1);

	CATCH_ASSERT(
		"SocketInterface::getNetworkInterfaces_Nix(2)\n",
		gniNixLinkTraverse(list, addresses);
	);
	freeifaddrs(addresses);
#endif
	return list;
}


void Internal::gniNixLinkTraverse(
	std::vector<NetInterface>& list, struct ifaddrs* addresses) {
#ifndef _MSC_VER
	// WARNING: ifa_addr may be NULL
	for(auto target = addresses; target != NULL; target = target->ifa_next) {
		NetInterface token;

		token.flags     = target->ifa_flags;
		token.name      = std::string(target->ifa_name);
		token.address   = sockAddr2String(target->ifa_addr);
		token.netmask   = sockAddr2String(target->ifa_netmask);
		token.broadcast = sockAddr2String(target->ifa_broadaddr);
		token.ipv4      = (target->ifa_addr!=NULL)?
				(target->ifa_addr->sa_family == AF_INET):false;

#if defined(__APPLE__)
		if(target->ifa_addr != NULL) {
				struct sockaddr_dl* sdl = (struct sockaddr_dl*)target->ifa_addr;
				token.type = gniOSXGetInterfaceType(sdl->sdl_type);
		}
		else token.type = InterfaceType::OTHER;
#else
		switch(target->ifa_addr->sa_family) {
		case AF_INET: {
			CATCH_ASSERT(
				"SocketInterface::gniNixLinkTraverse(1)\n",
				token.type = gniLinuxGetInterfaceType(SocketDomain::INET, token.name);
			);
			break;
		}
		case AF_INET6: {
			CATCH_ASSERT(
				"SocketInterface::gniNixGetInterfaceType(2)\n",
				token.type = gniLinuxGetInterfaceType(SocketDomain::INET6, token.name);
			);
			break;
		}
		default: token.type = InterfaceType::OTHER;
		}
#endif

		list.push_back(token);
	}
#else
	UNUSED(list);
	UNUSED(addresses);
#endif
}


InterfaceType Internal::gniLinuxGetInterfaceType(SocketDomain domain,
	const std::string& interfaceName) {
#if defined(__linux__)
	SocketHandle handle;
	InterfaceType type;
	CATCH_ASSERT(
		"SocketInterface::gniNixGetInterfaceType(1)\n",
		handle = SocketInterface::create(domain, SocketType::DATAGRAM,
		    SocketProtocol::DEFAULT);
	);

	struct ifreq request;
	std::memcpy(
		request.ifr_name,
		(void*)&interfaceName[0],
		interfaceName.length() + 1
	);
	auto status = ::ioctl(handle.descriptor, SIOCGIFHWADDR, &request);

	std::ostringstream os;
	if(status == -1) {
		os << "SocketInterface::gniNixGetInterfaceType(2)\n";
		os << Internal::getErrorMessage() << " " << interfaceName << std::endl;
		try { SocketInterface::close(handle); }
		catch(std::runtime_error e) { os << e.what(); }
		throw std::runtime_error(os.str());
	}

	type = gniLinuxGetInterfaceType(request.ifr_hwaddr.sa_family);

	CATCH_ASSERT(
		"SocketInterface::gniNixGetInterfaceType(3)\n",
		SocketInterface::close(handle);
	);
	return type;
#else
	UNUSED(domain);
	UNUSED(interfaceName);
	return InterfaceType::OTHER;
#endif
}


InterfaceType Internal::gniLinuxGetInterfaceType(unsigned short family) {
#if defined(__linux__)
	switch(family) {
	case ARPHRD_ETHER:
	case ARPHRD_EETHER:             return InterfaceType::ETHERNET;
	case ARPHRD_IEEE802:
	case ARPHRD_IEEE802_TR:
	case ARPHRD_IEEE80211:
	case ARPHRD_IEEE80211_PRISM:
	case ARPHRD_IEEE80211_RADIOTAP: return InterfaceType::WIFI;
	case ARPHRD_IEEE1394:           return InterfaceType::FIREWIRE;
	case ARPHRD_PPP:                return InterfaceType::PPP;
	case ARPHRD_ATM:                return InterfaceType::ATM;
	default:                        return InterfaceType::OTHER;
	}
#else
	UNUSED(family);
	return InterfaceType::OTHER;
#endif
}


InterfaceType Internal::gniOSXGetInterfaceType(unsigned short family) {
#if defined(__APPLE__)
	switch(family) {
	case IFT_ETHER:
	case IFT_XETHER:    return InterfaceType::ETHERNET;
	// this is actually a Metropolitan Area Network (MAN)
	// but it can be used for WiFi interfaces
	case IFT_ISO88026:	return InterfaceType::WIFI;
	case IFT_IEEE1394:  return InterfaceType::FIREWIRE;
	case IFT_PPP:       return InterfaceType::PPP;
	case IFT_ATM:       return InterfaceType::ATM;
	default:            return InterfaceType::OTHER;
	}
#else
	UNUSED(family);
	return InterfaceType::OTHER;
#endif
}
