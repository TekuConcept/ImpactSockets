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
#if defined(__WINDOWS__)
    void traverseAdapters(std::vector<NetInterface>&,
        PIP_ADAPTER_ADDRESSES);
	void traverseUnicast(std::vector<NetInterface>&, NetInterface,
	    PIP_ADAPTER_UNICAST_ADDRESS);
	InterfaceType getInterfaceType(unsigned int);
#else /* NIX */
	void traverseLinks(std::vector<NetInterface>&, struct ifaddrs*);
	InterfaceType getInterfaceType(unsigned short);
#if defined(__LINUX__)
	InterfaceType getInterfaceType(SocketDomain, const std::string&);
#endif /* __LINUX__ */
#endif /* __WINDOWS__ */
}}


using namespace Impact;


Networking::NetInterface::NetInterface() :
	flags(0), name(""), address(""), netmask(""),
	broadcast(""), type(InterfaceType::OTHER), ipv4(false) {}


#if defined(__WINDOWS__)

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
|  WINDOWS FUNCTIONS                                                          |
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    std::vector<NetInterface> Networking::findNetworkInterfaces() {
    	std::vector<NetInterface> list;
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
    				"Networking::findNetworkInterfaces(1)\n"
    				"Memory allocation failed.");
    		}
    		status = GetAdaptersAddresses(AF_UNSPEC, flags, NULL,
    		    adapterAddresses, &size);
    		if (status == ERROR_BUFFER_OVERFLOW) {
    			free(adapterAddresses);
    			adapterAddresses = NULL;
    			retries++;
    		}
    		else break;
    	} while ((status == ERROR_BUFFER_OVERFLOW) && (retries < MAX_RETRY));
    
    	if (retries >= MAX_RETRY) {
    		throw std::runtime_error(
    			"Networking::findNetworkInterfaces(2)\n"
    			"Failed to identify buffer size for Adapter Addresses.");
    	}
    
    	Internal::traverseAdapters(list, adapterAddresses);
    	free(adapterAddresses);
    	return list;
    }


    void Internal::traverseAdapters(std::vector<NetInterface>& list,
        PIP_ADAPTER_ADDRESSES adapters) {
    	for (PIP_ADAPTER_ADDRESSES adapter = adapters;
    		adapter != NULL; adapter = adapter->Next) {
    		NetInterface token;
    		token.name  = toNarrowString(adapter->FriendlyName);
    		token.flags = (unsigned int)adapter->Flags;
    		token.type  = getInterfaceType(adapter->IfType);
    		traverseUnicast(list, token, adapter->FirstUnicastAddress);
    		list.push_back(token);
    	}
    }
    
    
    void Internal::traverseUnicast(std::vector<NetInterface>& list,
    	NetInterface token, PIP_ADAPTER_UNICAST_ADDRESS addresses) {
    	for (PIP_ADAPTER_UNICAST_ADDRESS address = addresses;
    		address != NULL; address = address->Next) {
    
    		auto socketAddress = address->Address.lpSockaddr;
    		token.address      = sockAddr2String(socketAddress);
    
    		if (socketAddress->sa_family == AF_INET) {
    			token.ipv4 = true;
    
    			struct sockaddr_in mask;
    			mask.sin_family = socketAddress->sa_family;
    			ConvertLengthToIpv4Mask(
    				address->OnLinkPrefixLength,
    				(PULONG)&mask.sin_addr
    			);
    			
    			token.netmask        = sockAddr2String((struct sockaddr*)&mask);
    			struct sockaddr_in broadcast;
    			broadcast.sin_family = socketAddress->sa_family;
    			unsigned long A      = *(unsigned long*)&mask.sin_addr;
    			unsigned long B      = *(unsigned long*)&(
    				(struct sockaddr_in*)socketAddress)->sin_addr;
    			unsigned long C      = (A&B | ~A);
    			broadcast.sin_addr   = *(struct in_addr*)&C;
    			token.broadcast      =
    			    sockAddr2String((struct sockaddr*)&broadcast);
    		}
    		// TODO: IPv6 mask and broadcast
    
    		list.push_back(token);
    	}
    }


    InterfaceType Internal::getInterfaceType(unsigned int code) {
    	switch (code) {
    	case IF_TYPE_ETHERNET_CSMACD: return InterfaceType::ETHERNET;
    	case IF_TYPE_IEEE80211:       return InterfaceType::WIFI;
    	case IF_TYPE_IEEE1394:        return InterfaceType::FIREWIRE;
    	case IF_TYPE_PPP:             return InterfaceType::PPP;
    	case IF_TYPE_ATM:             return InterfaceType::ATM;
    	default:                      return InterfaceType::OTHER;
    	}
    }

#else /* NIX */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
|  LINUX | OSX FUNCTIONS                                                      |
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    std::vector<NetInterface> Networking::findNetworkInterfaces() {
    	std::vector<NetInterface> list;
    	struct ::ifaddrs* addresses;
    	auto status = ::getifaddrs(&addresses);
    
    	ASSERT("Networking::findNetworkInterfaces(1)\n", status == -1);
    
    	CATCH_ASSERT(
    		"Networking::findNetworkInterfaces(2)\n",
    		Internal::traverseLinks(list, addresses);
    	);
    	freeifaddrs(addresses);
    	return list;
    }
    
    
    void Internal::traverseLinks(std::vector<NetInterface>& list,
        struct ifaddrs* addresses) {
    	// WARNING: ifa_addr may be NULL
    	for(auto target = addresses; target != NULL; target = target->ifa_next){
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
				token.type = getInterfaceType(sdl->sdl_type);
    		}
    		else token.type = InterfaceType::OTHER;
        #else
    		switch(target->ifa_addr->sa_family) {
    		case AF_INET: {
    			CATCH_ASSERT(
    				"Networking::traverseLinks(1)\n",
    				token.type = getInterfaceType(SocketDomain::INET, token.name);
    			);
    			break;
    		}
    		case AF_INET6: {
    			CATCH_ASSERT(
    				"Networking::traverseLinks(2)\n",
    				token.type = getInterfaceType(SocketDomain::INET6, token.name);
    			);
    			break;
    		}
    		default: token.type = InterfaceType::OTHER;
	        }
        #endif

		list.push_back(token);
	}
}


#if defined(__LINUX__)

    InterfaceType Internal::getInterfaceType(SocketDomain domain,
    	const std::string& interfaceName) {
    	SocketHandle handle;
    	InterfaceType type;
    	CATCH_ASSERT(
    		"Networking::getInterfaceType(1)\n",
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
    		os << "Networking::getInterfaceType(2)\n";
    		os << Internal::getErrorMessage() << " " << interfaceName << std::endl;
    		try { SocketInterface::close(handle); }
    		catch(std::runtime_error e) { os << e.what(); }
    		throw std::runtime_error(os.str());
    	}
    
    	type = getInterfaceType(request.ifr_hwaddr.sa_family);
    
    	CATCH_ASSERT(
    		"Networking::getInterfaceType(3)\n",
    		SocketInterface::close(handle);
    	);
    	return type;
    }
    
    
    InterfaceType Internal::getInterfaceType(unsigned short family) {
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
    }

#endif /* __LINUX__ */


#if defined(__APPLE__)

    InterfaceType Internal::gniOSXGetInterfaceType(unsigned short family) {
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
    }

#endif /* __APPLE__ */

#endif /* __WINDOWS__ */
