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

#if defined(__WINDOWS__)
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

#if defined(__LINUX__)
	#include <net/if_arp.h>
#endif

#if defined(__WINDOWS__)
	#define CLOSE_SOCKET(x) closesocket(x)
	#define SOC_POLL WSAPoll
	#define CCHAR_PTR const char*
	#define CHAR_PTR char*

	#undef ASSERT
	#pragma warning(disable:4996)
	#pragma comment (lib, "Ws2_32.lib")
	#pragma comment (lib, "Mswsock.lib")
	#pragma comment (lib, "AdvApi32.lib")
	#pragma comment (lib, "IPHLPAPI.lib")
#else
	#define CLOSE_SOCKET(x) ::close(x)
	#define SOC_POLL ::poll
	#define CCHAR_PTR void*
	#define CHAR_PTR void*

 	#define SOCKET_ERROR -1
 	#define INVALID_SOCKET -1
#endif

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


using namespace Impact;


KeepAliveOptions::KeepAliveOptions() :
	enabled(false), idleTime(7200000),
	interval(1000), retries(5) {}


SocketInterface::SocketInterface() {}


SocketHandle SocketInterface::create(SocketDomain domain, SocketType socketType,
	SocketProtocol protocol) {
#if defined(__WINDOWS__)
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
#if defined(__WINDOWS__)
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
	unsigned short port) {
	sockaddr_in socketAddress;
	::memset(&socketAddress, 0, sizeof(socketAddress));
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	socketAddress.sin_port = htons(port);
	auto status = ::bind(handle.descriptor, (sockaddr*)&socketAddress,
		sizeof(sockaddr_in));

	ASSERT("SocketInterface::setLocalPort()\n", status == SOCKET_ERROR);
}


void SocketInterface::setLocalAddressAndPort(const SocketHandle& handle,
	const std::string& address, unsigned short port) {
	sockaddr_in socketAddress;

	CATCH_ASSERT(
		"SocketInterface::setLocalAddressAndPort(1)\n",
		Internal::fillAddress(handle.domain, handle.type, handle.protocol,
			address, port, socketAddress);
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
		Internal::fillAddress(handle.domain, handle.type, handle.protocol,
			address, port, destinationAddress);
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
		Internal::fillAddress(handle.domain, handle.type, handle.protocol,
			address, port, destinationAddress);
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
	port    = ntohs(clientAddress.sin_port);

	return status;
}


void SocketInterface::keepalive(const SocketHandle& handle,
	KeepAliveOptions options) {
	// http://helpdoco.com/C++-C/how-to-use-tcp-keepalive.htm
	std::ostringstream os("SocketInterface::keepalive()\n");
	auto errors = 0;
	auto status = 0;
#if defined(__WINDOWS__)
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
