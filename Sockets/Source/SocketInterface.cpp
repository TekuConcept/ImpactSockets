/**
 * Created by TekuConcept on June 15, 2018
 */

#include "SocketInterface.h"

#include <sys/types.h>       // For data types#include <cstring>           // For strerror and memset
#include <stdlib.h>          // For atoi
#include <errno.h>           // For errno
#include <stdexcept>         // For std::runtime_error
#include <sstream>

#if defined(_MSC_VER)
	#include <ws2tcpip.h>
 	#include <mstcpip.h>     // struct tcp_keepalive
#else
	#include <sys/socket.h>  // For socket(), connect(), send(), and recv()
	#include <netdb.h>       // For gethostbyname()
	#include <arpa/inet.h>   // For inet_addr(), ntohs()
	#include <unistd.h>      // For close()
 	#include <netinet/tcp.h> // For IPPROTO_TCP, TCP_KEEPCNT, TCP_KEEPINTVL,
 							 // TCP_KEEPIDLE
#endif

#if defined(_MSC_VER)
	#define SOC_POLL WSAPoll
	#define CLOSE_SOCKET(x) closesocket(x)
	#define SOC_SD_HOW SD_BOTH
	#define CCHAR_PTR const char *
	#define CHAR_PTR char *

	#pragma warning(disable:4996)
	#pragma comment (lib, "Ws2_32.lib")
	#pragma comment (lib, "Mswsock.lib")
	#pragma comment (lib, "AdvApi32.lib")
#else
	#define SOC_POLL ::poll
	#define CLOSE_SOCKET(x) ::close(x)
	#define SOC_SD_HOW SHUT_RDWR
	#define CCHAR_PTR raw_type *
	#define CHAR_PTR raw_type *

 	#define SOCKET_ERROR -1
#endif

typedef void raw_type;       // Type used for raw data on this platform

using namespace Impact;


KeepAliveOptions::KeepAliveOptions() :
	enabled(false), idleTime(7200000),
	interval(1000), retries(5) {}


SocketInterface::SocketInterface() {}


std::string SocketInterface::getErrorMessage() {
	std::ostringstream os;
#if defined(_MSC_VER)
	char data[128];
	auto errorCode = WSAGetLastError();
	auto status = FromatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		data,
		128,
		NULL
	);
	if(status == 0) os << "[No Error Message Available]";
	else os << data;
#else
	os << strerror(errno);
#endif
	return os.str();
}


std::string SocketInterface::getHostErrorMessage() {
#if defined(_MSC_VER)
	return getErrorMessage();
#else
	std::ostringstream os;
	switch(h_errno) {
	case HOST_NOT_FOUND: os << "[Host Error] Host Not Found"; break;
	case NO_DATA:        os << "[Host Error] No Data"; break;
	case NO_RECOVERY:    os << "[Host Error] No Recovery"; break;
	case TRY_AGAIN:      os << "[Host Error] Try Again"; break;
	}
	return os.str();
#endif
}


std::string SocketInterface::getLocalAddress(const SocketHandle& handle) {
	sockaddr_in address;
	auto addressLength = sizeof(address);
	auto status = ::getsockname(handle.descriptor,
		(sockaddr*)&address, (socklen_t*)&addressLength);

	if(status == SOCKET_ERROR) {
		std::string message("SocketInterface::getLocalAddress() ");
		message.append(getErrorMessage());
		throw std::runtime_error(message);
	}
	return inet_ntoa(address.sin_addr);
}


unsigned short SocketInterface::getLocalPort(const SocketHandle& handle) {
	sockaddr_in address;
	auto addressLength = sizeof(address);
	auto status = ::getsockname(handle.descriptor,
		(sockaddr*)&address, (socklen_t*)&addressLength);

	if (status == SOCKET_ERROR) {
		std::string message("SocketInterface::getLocalPort() ");
		message.append(getErrorMessage());
		throw std::runtime_error(message);
	}
	return ntohs(address.sin_port);
}


void SocketInterface::setLocalPort(const SocketHandle& handle,
	unsigned short localPort) {
	// Bind the socket to its port
	sockaddr_in socketAddress;
	::memset(&socketAddress, 0, sizeof(socketAddress));
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	socketAddress.sin_port = htons(localPort);
	auto status = ::bind(handle.descriptor, (sockaddr*)&socketAddress,
		sizeof(sockaddr_in));

	if (status == SOCKET_ERROR) {
		std::string message("SocketInterface::setLocalPort() ");
		message.append(getErrorMessage());
		throw std::runtime_error(message);
	}
}


void SocketInterface::setLocalAddressAndPort(const SocketHandle& handle,
	const std::string& localAddress,
	unsigned short localPort) {
	// Get the address of the requested host
	sockaddr_in socketAddress;
	
	try { fillAddress(localAddress, localPort, socketAddress); }
	catch(std::runtime_error e) {
		std::string message(e.what());
		message.append("\nSocketInterface::setLocalAddressAndPort()");
		throw std::runtime_error(message);
	}

	auto status = ::bind(handle.descriptor, (sockaddr*)&socketAddress,
		sizeof(sockaddr_in));

	if (status == SOCKET_ERROR) {
		std::string message("SocketInterface::setLocalAddressAndPort() ");
		message.append(getErrorMessage());
		throw std::runtime_error(message);
	}
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


void SocketInterface::fillAddress(const std::string& address,
	unsigned short port, sockaddr_in& socketAddress) {
	memset(&socketAddress, 0, sizeof(socketAddress));
	socketAddress.sin_family = AF_INET; // Internet address
	hostent* host = ::gethostbyname(address.c_str());

	if (host == NULL) {
		std::string message("SocketInterface::fillAddress() ");
		message.append(getHostErrorMessage());
		throw std::runtime_error(message);
	}

	socketAddress.sin_addr.s_addr = *((unsigned long *)host->h_addr_list[0]);
	socketAddress.sin_port = htons(port); // Assign port in network byte order
}


std::string SocketInterface::getForeignAddress(const SocketHandle& handle) {
	sockaddr_in address;
	unsigned int addressLength = sizeof(address);
	auto status = ::getpeername(handle.descriptor, (sockaddr*)&address,
		(socklen_t*)&addressLength);

	if (status == SOCKET_ERROR) {
		std::string message("SocketInterface::getForeignAddress() ");
		message.append(getErrorMessage());
		throw std::runtime_error(message);
	}

	return inet_ntoa(address.sin_addr);
}


unsigned short SocketInterface::getForeignPort(const SocketHandle& handle) {
	sockaddr_in address;
	unsigned int addressLength = sizeof(address);
	auto status = getpeername(handle.descriptor, (sockaddr*)&address,
		(socklen_t*)&addressLength);

	if (status == SOCKET_ERROR) {
		std::string message("SocketInterface::getForeignPort() ");
		message.append(getErrorMessage());
		throw std::runtime_error(message);
	}

	return ntohs(address.sin_port);
}


void SocketInterface::send(const SocketHandle& handle, const void* buffer,
	int bufferLen, MessageFlags flags) {
	auto status = ::send(handle.descriptor, (CCHAR_PTR)buffer, bufferLen,
		(int)flags);

	if (status == SOCKET_ERROR) {
		std::string message("SocketInterface::send() ");
		message.append(getErrorMessage());
		throw std::runtime_error(message);
	}
}


int SocketInterface::recv(const SocketHandle& handle, void* buffer,
	int bufferLen, MessageFlags flags) {
	int status = ::recv(handle.descriptor, (CHAR_PTR)buffer, bufferLen,
		(int)flags);

	if (status == SOCKET_ERROR) {
		std::string message("SocketInterface::recv() ");
		message.append(getErrorMessage());
		throw std::runtime_error(message);
	}

	return status; /* number of bytes received or EOF */
}


void SocketInterface::keepalive(const SocketHandle& handle,
	KeepAliveOptions options) {
    // http://helpdoco.com/C++-C/how-to-use-tcp-keepalive.htm
    std::ostringstream os("SocketInterface::keepalive() ");
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
		os << getErrorMessage();
		throw std::runtime_error(os.str());
	}
#else /* UNIX|LINUX */
	status = setsockopt(handle.descriptor, SOL_SOCKET, SO_KEEPALIVE,
		(const char*)&options.enabled, sizeof(int));
    if(status == SOCKET_ERROR) {
        os << "[keepalive] ";
        os << getErrorMessage();
        os << std::endl;
        errors |= 1;
    }
#ifndef __APPLE__
    status = setsockopt(handle.descriptor, IPPROTO_TCP, TCP_KEEPIDLE,
		(const char*)&options.idleTime, sizeof(int));
    if(status == SOCKET_ERROR) {
        os << "[idle] ";
        os << getErrorMessage();
        os << std::endl;
        errors |= 8;
    }
#endif /* __APPLE__ */
    status = setsockopt(handle.descriptor, IPPROTO_TCP, TCP_KEEPINTVL,
        (const char*)&options.interval, sizeof(int));
    if(status == SOCKET_ERROR) {
        os << "[interval] ";
        os << getErrorMessage();
        os << std::endl;
        errors |= 2;
    }
#endif /* UNIX|LINUX */
    status = setsockopt(handle.descriptor, IPPROTO_TCP, TCP_KEEPCNT,
		(const char*)&options.retries, sizeof(int));
    if(status == SOCKET_ERROR) {
        os << "[count] ";
        os << getErrorMessage();
        os << std::endl;
        errors |= 4;
    }
    if(errors) throw std::runtime_error(os.str());
}

