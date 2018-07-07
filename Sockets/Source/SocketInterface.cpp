/**
 * Created by TekuConcept on June 15, 2018
 */

#include "SocketInterface.h"
#include "Generic.h"

#include <sys/types.h>			// For data types
#include <cstring>				// For strerror and memset
#include <stdlib.h>				// For atoi
#include <errno.h>				// For errno
#include <stdexcept>			// For std::runtime_error
#include <sstream>

#if defined(_MSC_VER)
	#pragma pop_macro("IN")     // pushed in SocketTypes.h
	#pragma pop_macro("OUT")    // pushed in SocketTypes.h
	#pragma pop_macro("ERROR")  // pushed in SocketTypes.h
	#include <ws2tcpip.h>
 	#include <mstcpip.h>		// struct tcp_keepalive
	#include <iphlpapi.h>
#else
	#include <sys/socket.h>		// For socket(), connect(), send(), and recv()
	#include <netdb.h>			// For gethostbyname()
	#include <arpa/inet.h>		// For inet_addr(), ntohs()
	#include <unistd.h>			// For close()
 	#include <netinet/tcp.h>	// For IPPROTO_TCP, TCP_KEEPCNT, TCP_KEEPINTVL,
 								// TCP_KEEPIDLE
    #include <sys/ioctl.h>      // For ioctl()
    #include <net/if.h>         // For ifconf
#if defined(__APPLE__)
	#include <net/if_types.h>   // For IFT_XXX types
	#include <net/if_dl.h>      // For sockaddr_dl
#endif
#endif

#if defined(__linux__)
	#include <net/if_arp.h>
#endif

#if defined(_MSC_VER)
	#define CLOSE_SOCKET(x) closesocket(x)
	#define SOC_POLL WSAPoll
	#define CCHAR_PTR const char*
	#define CHAR_PTR char*

	#undef ASSERT
	#pragma warning(disable:4996)
	#pragma comment (lib, "Ws2_32.lib")
	#pragma comment (lib, "Mswsock.lib")
	#pragma comment (lib, "AdvApi32.lib")
	#pragma comment(lib, "IPHLPAPI.lib")
#else
	#define CLOSE_SOCKET(x) ::close(x)
	#define SOC_POLL ::poll
	#define CCHAR_PTR void*
	#define CHAR_PTR void*

 	#define SOCKET_ERROR -1
 	#define INVALID_SOCKET -1
#endif

#define UNUSED(x) (void)x

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

#define WIN_ASSERT(title,cond,error,fin)\
	if(cond) {\
		fin\
		std::string message( title );\
		message.append(Internal::getWinErrorMessage( error ));\
		throw std::runtime_error(message);\
	}

#include <iostream>
#include <iomanip>
#define VERBOSE(x) std::cout << x << std::endl

using namespace Impact;



KeepAliveOptions::KeepAliveOptions() :
	enabled(false), idleTime(7200000),
	interval(1000), retries(5) {}


NetInterface::NetInterface() :
	flags(0), name(""), address(""), netmask(""),
	broadcast(""), type(InterfaceType::OTHER), ipv4(false) {}


SocketInterface::SocketInterface() {}


std::string SocketInterface::sockAddr2String(const struct sockaddr* address) {
	if(!address) return "";
	switch(address->sa_family) {
		case AF_INET: {
			char buffer[INET_ADDRSTRLEN];
			struct sockaddr_in* socketAddress = (struct sockaddr_in*)address;
			auto result = inet_ntop(AF_INET, &socketAddress->sin_addr,
				buffer, INET_ADDRSTRLEN);
			if(result == NULL) return "";
			else return std::string(result);
		}
		case AF_INET6: {
			char buffer[INET6_ADDRSTRLEN];
			struct sockaddr_in6* socketAddress = (struct sockaddr_in6*)address;
			auto result = inet_ntop(AF_INET6, &socketAddress->sin6_addr,
				buffer, INET6_ADDRSTRLEN);
			if(result == NULL) return "";
			else return std::string(result);
			return std::string(buffer);
		}
		default: return "";
	};
}


SocketHandle SocketInterface::create(SocketDomain domain, SocketType socketType,
	SocketProtocol protocol) {
#if defined(_MSC_VER)
	static WSADATA wsaData;
	auto status = WSAStartup(MAKEWORD(2, 2), &wsaData);
	WIN_ASSERT("SocketInterface::create(1)\n", status != 0, status, (void)0;);
#endif
	SocketHandle handle;
	handle.domain     = domain;
	handle.type       = socketType;
	handle.protocol   = protocol;
	handle.descriptor = ::socket((int)domain, (int)socketType, (int)protocol);
	ASSERT(
		"SocketInterface::create(2)\n",
		handle.descriptor == INVALID_SOCKET
	);
	return handle;
}


void SocketInterface::close(SocketHandle& handle) {
	auto status = CLOSE_SOCKET(handle.descriptor);

	ASSERT("SocketInterface::close()\n", status == SOCKET_ERROR);

	handle.descriptor = INVALID_SOCKET;
#if defined(_MSC_VER)
	WSACleanup();
#endif
}


std::string SocketInterface::getLocalAddress(const SocketHandle& handle) {
	sockaddr_in address;
	auto addressLength = sizeof(address);
	auto status = ::getsockname(handle.descriptor,
		(sockaddr*)&address, (socklen_t*)&addressLength);

	ASSERT("SocketInterface::getLocalAddress()\n", status == SOCKET_ERROR);

	return inet_ntoa(address.sin_addr);
}


unsigned short SocketInterface::getLocalPort(const SocketHandle& handle) {
	sockaddr_in address;
	auto addressLength = sizeof(address);
	auto status = ::getsockname(handle.descriptor,
		(sockaddr*)&address, (socklen_t*)&addressLength);

	ASSERT("SocketInterface::getLocalPort()\n", status == SOCKET_ERROR);

	return ntohs(address.sin_port);
}


void SocketInterface::setLocalPort(const SocketHandle& handle,
	unsigned short localPort) {
	sockaddr_in socketAddress;
	::memset(&socketAddress, 0, sizeof(socketAddress));
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	socketAddress.sin_port = htons(localPort);
	auto status = ::bind(handle.descriptor, (sockaddr*)&socketAddress,
		sizeof(sockaddr_in));

	ASSERT("SocketInterface::setLocalPort()\n", status == SOCKET_ERROR);
}


void SocketInterface::setLocalAddressAndPort(const SocketHandle& handle,
	const std::string& localAddress,
	unsigned short localPort) {
	sockaddr_in socketAddress;

	CATCH_ASSERT(
		"SocketInterface::setLocalAddressAndPort(1)\n",
		Internal::fillAddress(handle, localAddress, localPort, socketAddress);
	);

	auto status = ::bind(handle.descriptor, (sockaddr*)&socketAddress,
		sizeof(sockaddr_in));

	ASSERT("SocketInterface::setLocalAddressAndPort(2)\n", status == SOCKET_ERROR);
}


void SocketInterface::setBroadcast(const SocketHandle& handle, bool enabled) {
	auto permission = enabled?1:0;
	auto status = setsockopt(handle.descriptor, SOL_SOCKET, SO_BROADCAST,
		(CCHAR_PTR)&permission, sizeof(permission));

	ASSERT("SocketInterface::setBroadcast()\n", status == SOCKET_ERROR);
}


void SocketInterface::setMulticastTTL(const SocketHandle& handle,
	unsigned char ttl) {
	auto status = setsockopt(handle.descriptor, IPPROTO_IP, IP_MULTICAST_TTL,
		(CCHAR_PTR)&ttl, sizeof(ttl));

	ASSERT("SocketInterface::setMulticastTTL()\n", status == SOCKET_ERROR);
}


unsigned short SocketInterface::resolveService(const std::string& service,
	const std::string& protocol) {
	struct servent* serviceInfo = ::getservbyname(service.c_str(),
		protocol.c_str());

	if (serviceInfo == NULL)
		return static_cast<unsigned short>(atoi(service.c_str()));
		/* Service is port number */
	else return ntohs(serviceInfo->s_port);
		/* Found port (network byte order) by name */
}


std::string SocketInterface::getForeignAddress(const SocketHandle& handle) {
	sockaddr_in address;
	unsigned int addressLength = sizeof(address);
	auto status = ::getpeername(handle.descriptor, (sockaddr*)&address,
		(socklen_t*)&addressLength);

	ASSERT("SocketInterface::getForeignAddress()\n", status == SOCKET_ERROR);

	return inet_ntoa(address.sin_addr);
}


unsigned short SocketInterface::getForeignPort(const SocketHandle& handle) {
	sockaddr_in address;
	unsigned int addressLength = sizeof(address);
	auto status = getpeername(handle.descriptor, (sockaddr*)&address,
		(socklen_t*)&addressLength);

	ASSERT("SocketInterface::getForeignPort()\n", status == SOCKET_ERROR);

	return ntohs(address.sin_port);
}


void SocketInterface::connect(const SocketHandle& handle,
	unsigned short port, const std::string& address) {
	sockaddr_in destinationAddress;

	CATCH_ASSERT(
		"SocketInterface::connect(1)\n",
		Internal::fillAddress(handle, address, port, destinationAddress);
	);

	auto status = ::connect(handle.descriptor, (sockaddr*)&destinationAddress,
		sizeof(destinationAddress));

	ASSERT("SocketInterface::connect(2)\n", status == SOCKET_ERROR);
}


void SocketInterface::listen(const SocketHandle& handle, int backlog) {
	auto status = ::listen(handle.descriptor, backlog);

	ASSERT("SocketInterface::listen()\n", status == SOCKET_ERROR);
}


void SocketInterface::shutdown(const SocketHandle& handle,
	SocketChannel channel) {
	auto status = ::shutdown(handle.descriptor, (int)channel);

	ASSERT("SocketInterface::shutdown()\n", status == SOCKET_ERROR);
}


void SocketInterface::accept(const SocketHandle& handle, SocketHandle& peer) {
	peer.descriptor = ::accept(handle.descriptor, NULL, NULL);

	ASSERT("SocketInterface::accept()\n", peer.descriptor == INVALID_SOCKET);
}


void SocketInterface::group(const SocketHandle& handle,
	std::string multicastName, GroupApplication method) {
	struct ip_mreq multicastRequest;
	multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastName.c_str());
	multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
	auto status = setsockopt(handle.descriptor, IPPROTO_IP, (int)method,
		(CCHAR_PTR)&multicastRequest, sizeof(multicastRequest));

	ASSERT("SocketInterface::group()\n", status == SOCKET_ERROR);
}


void SocketInterface::send(const SocketHandle& handle, const void* buffer,
	int bufferLen, MessageFlags flags) {
	auto status = ::send(handle.descriptor, (CCHAR_PTR)buffer, bufferLen,
		(int)flags);

	ASSERT("SocketInterface::send()\n", status == SOCKET_ERROR);
}


int SocketInterface::sendto(const SocketHandle& handle, const void* buffer,
	int length, unsigned short port, const std::string& address,
	MessageFlags flags) {
	sockaddr_in destinationAddress;

	CATCH_ASSERT(
		"SocketInterface::sendto(1)\n",
		Internal::fillAddress(handle, address, port, destinationAddress);
	);

	auto status = ::sendto(handle.descriptor, (CCHAR_PTR)buffer, length,
		(int)flags, (sockaddr*)&destinationAddress, sizeof(destinationAddress));

	ASSERT("SocketInterface::sendto(2)\n", status == SOCKET_ERROR);

	return status;
}


int SocketInterface::recv(const SocketHandle& handle, void* buffer,
	int bufferLen, MessageFlags flags) {
	int status = ::recv(handle.descriptor, (CHAR_PTR)buffer, bufferLen,
		(int)flags);

	ASSERT("SocketInterface::recv()\n", status == SOCKET_ERROR);

	return status; /* number of bytes received or EOF */
}


int SocketInterface::recvfrom(const SocketHandle& handle, void* buffer,
	int length, unsigned short& port, std::string& address, MessageFlags flags){
	sockaddr_in clientAddress;
	socklen_t addressLength = sizeof(clientAddress);
	auto status = ::recvfrom(handle.descriptor, (CHAR_PTR)buffer, length,
		(int)flags, (sockaddr*)&clientAddress, (socklen_t*)&addressLength);

	ASSERT("SocketInterface::recvfrom()\n", status == SOCKET_ERROR);

	address = inet_ntoa(clientAddress.sin_addr);
	port = ntohs(clientAddress.sin_port);

	return status;
}


void SocketInterface::keepalive(const SocketHandle& handle,
	KeepAliveOptions options) {
	// http://helpdoco.com/C++-C/how-to-use-tcp-keepalive.htm
	std::ostringstream os("SocketInterface::keepalive()\n");
	auto errors = 0;
	auto status = 0;
#if defined(_MSC_VER)
	DWORD bytesReturned = 0;
	struct tcp_keepalive config;
	config.onoff = options.enabled;
	config.keepalivetime = options.idleTime;
	config.keepaliveinterval = options.interval;
	status = WSAIoctl(handle.descriptor, SIO_KEEPALIVE_VALS, &config,
		sizeof(config), NULL, 0, &bytesReturned, NULL, NULL);
	if (status == SOCKET_ERROR) {
		os << Internal::getErrorMessage();
		throw std::runtime_error(os.str());
	}
#else /* OSX|LINUX */
	status = setsockopt(handle.descriptor, SOL_SOCKET, SO_KEEPALIVE,
		(const char*)&options.enabled, sizeof(int));
	if(status == SOCKET_ERROR) {
		os << "[keepalive] ";
		os << Internal::getErrorMessage();
		os << std::endl;
		errors |= 1;
	}
#ifndef __APPLE__
	status = setsockopt(handle.descriptor, IPPROTO_TCP, TCP_KEEPIDLE,
		(const char*)&options.idleTime, sizeof(int));
	if(status == SOCKET_ERROR) {
		os << "[idle] ";
		os << Internal::getErrorMessage();
		os << std::endl;
		errors |= 8;
	}
#endif /* __APPLE__ */
	status = setsockopt(handle.descriptor, IPPROTO_TCP, TCP_KEEPINTVL,
		(const char*)&options.interval, sizeof(int));
	if(status == SOCKET_ERROR) {
		os << "[interval] ";
		os << Internal::getErrorMessage();
		os << std::endl;
		errors |= 2;
	}
#endif /* UNIX|LINUX */
	status = setsockopt(handle.descriptor, IPPROTO_TCP, TCP_KEEPCNT,
		(const char*)&options.retries, sizeof(int));
	if(status == SOCKET_ERROR) {
		os << "[count] ";
		os << Internal::getErrorMessage();
		os << std::endl;
		errors |= 4;
	}
	if(errors) throw std::runtime_error(os.str());
}


int SocketInterface::select(
	std::vector<SocketHandle*> readHandles,
	std::vector<SocketHandle*> writeHandles,
	int timeout, unsigned int microTimeout) {
	struct timeval time_s;
	time_s.tv_sec = (unsigned int)(0xFFFFFFFF&timeout);
	time_s.tv_usec = microTimeout;

	fd_set readSet, writeSet;
	unsigned int nfds = 0;

	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);

	for(const auto& handle : readHandles) {
		unsigned int descriptor = handle->descriptor;
		FD_SET(descriptor, &readSet);
		if (descriptor > nfds) nfds = descriptor;
	}

	for(const auto& handle : writeHandles) {
		unsigned int descriptor = handle->descriptor;
		FD_SET(descriptor, &writeSet);
		if (descriptor > nfds) nfds = descriptor;
	}

	auto status = ::select(nfds + 1, &readSet, &writeSet, NULL,
		((timeout<0)?NULL:&time_s));

	ASSERT("SocketInterface::select()\n", status == SOCKET_ERROR);

	return status;
}


int SocketInterface::poll(SocketPollTable& token, int timeout) {
	/* timeout: -1 blocking, 0 nonblocking, 0> timeout */
	struct pollfd* fds = token._descriptors_.data();
	auto size = token.size();
	auto status = SOC_POLL(fds, size, timeout);

	ASSERT("SocketInterface::poll()\n", status == SOCKET_ERROR);

	/* status: -1 error, 0 timeout, 0> success */
	return status;
}


std::vector<NetInterface> SocketInterface::getNetworkInterfaces() {
#if defined(_MSC_VER)
	CATCH_ASSERT(
		"SocketInterface::getNetworkInterfaces()\n",
		return getNetworkInterfaces_Win();
	);
#else /* OSX|LINUX */
	CATCH_ASSERT(
		"SocketInterface::getNetworkInterfaces()\n",
		return getNetworkInterfaces_Nix();
	);
#endif
}


std::vector<NetInterface> SocketInterface::getNetworkInterfaces_Win() {
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

	/*INTERFACE_INFO info[64];
	int length = 0;

	CATCH_ASSERT(
		"SocketInterface::getNetworkInterfaces_Win()\n",
		gniWinNetProbe((void*)&info, sizeof(info), length);
	);

	for(int i = 0; i < length; i++) {
		char buffer[INET_ADDRSTRLEN];
		NetInterface token;
		struct sockaddr_in address   = info[i].iiAddress.AddressIn;
		struct sockaddr_in netmask   = info[i].iiNetmask.AddressIn;
		struct sockaddr_in broadcast = info[i].iiBroadcastAddress.AddressIn;
		token.flags                  = (unsigned int)info[i].iiFlags;

		auto result = inet_ntop(address.sin_family, &address.sin_addr.s_addr,
			buffer, INET_ADDRSTRLEN);
		if(result == NULL) { token.address = ""; }
		else { token.address = std::string(result); }

		result = inet_ntop(netmask.sin_family, &netmask.sin_addr.s_addr,
			buffer, INET_ADDRSTRLEN);
		if(result == NULL) { token.netmask = ""; }
		else { token.netmask = std::string(result); }

		result = inet_ntop(broadcast.sin_family, &broadcast.sin_addr.s_addr,
			buffer, INET_ADDRSTRLEN);
		if(result == NULL) { token.broadcast = ""; }
		else { token.broadcast = std::string(result); }

		token.ipv4      = address.sin_family == AF_INET;
		list.push_back(token);
	}*/
#endif
	return list;
}


void SocketInterface::gniWinAdapterTraverse(
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


void SocketInterface::gniWinUnicastTraverse(std::vector<NetInterface>& list,
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


InterfaceType SocketInterface::gniWinGetInterfaceType(unsigned int code) {
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


std::vector<NetInterface> SocketInterface::getNetworkInterfaces_Nix() {
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


void SocketInterface::gniNixLinkTraverse(
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


InterfaceType SocketInterface::gniLinuxGetInterfaceType(SocketDomain domain,
	const std::string& interfaceName) {
#if defined(__linux__)
	SocketHandle handle;
	InterfaceType type;
	CATCH_ASSERT(
		"SocketInterface::gniNixGetInterfaceType(1)\n",
		handle = create(domain, SocketType::DATAGRAM, SocketProtocol::DEFAULT);
	);

	struct ifreq request;
	std::memcpy(
		request.ifr_name,
		(void*)&interfaceName[0],
		interfaceName.length() + 1
	);
	auto status = ioctl(handle.descriptor, SIOCGIFHWADDR, &request);

	std::ostringstream os;
	if(status == -1) {
		os << "SocketInterface::gniNixGetInterfaceType(2)\n";
		os << Internal::getErrorMessage() << " " << interfaceName << std::endl;
		try { close(handle); }
		catch(std::runtime_error e) { os << e.what(); }
		throw std::runtime_error(os.str());
	}

	type = gniLinuxGetInterfaceType(request.ifr_hwaddr.sa_family);

	CATCH_ASSERT(
		"SocketInterface::gniNixGetInterfaceType(3)\n",
		close(handle);
	);
	return type;
#else
	UNUSED(domain);
	UNUSED(interfaceName);
	return InterfaceType::OTHER;
#endif
}


InterfaceType SocketInterface::gniLinuxGetInterfaceType(unsigned short family) {
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


InterfaceType SocketInterface::gniOSXGetInterfaceType(unsigned short family) {
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
