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
#else
	#include <sys/socket.h>  // For socket(), connect(), send(), and recv()
	#include <netdb.h>       // For gethostbyname()
	#include <arpa/inet.h>   // For inet_addr(), ntohs()
	#include <unistd.h>      // For close()
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


std::string SocketInterface::getLocalAddress(SocketHandle handle) {
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


unsigned short SocketInterface::getLocalPort(SocketHandle handle) {
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


void SocketInterface::setLocalPort(SocketHandle handle,
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


void SocketInterface::setLocalAddressAndPort(SocketHandle handle,
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