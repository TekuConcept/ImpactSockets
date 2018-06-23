/**
 * Created by TekuConcept on June 15, 2018
 */

#include "SocketInterface.h"

#include <sys/types.h>			// For data types
#include <cstring>				// For strerror and memset
#include <stdlib.h>				// For atoi
#include <errno.h>				// For errno
#include <stdexcept>			// For std::runtime_error
#include <sstream>

#if defined(_MSC_VER)
	#include <ws2tcpip.h>
 	#include <mstcpip.h>		// struct tcp_keepalive
#else
	#include <sys/socket.h>		// For socket(), connect(), send(), and recv()
	#include <netdb.h>			// For gethostbyname()
	#include <arpa/inet.h>		// For inet_addr(), ntohs()
	#include <unistd.h>			// For close()
 	#include <netinet/tcp.h>	// For IPPROTO_TCP, TCP_KEEPCNT, TCP_KEEPINTVL,
 								// TCP_KEEPIDLE
#endif

#if defined(_MSC_VER)
	#define CLOSE_SOCKET(x) closesocket(x)
	#define SOC_POLL WSAPoll
	#define CCHAR_PTR const char*
	#define CHAR_PTR char*

	#pragma warning(disable:4996)
	#pragma comment (lib, "Ws2_32.lib")
	#pragma comment (lib, "Mswsock.lib")
	#pragma comment (lib, "AdvApi32.lib")
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
 		message.append(getErrorMessage());\
 		throw std::runtime_error(message);\
 	}


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
	case HOST_NOT_FOUND:	os << "[Host Error] Host Not Found"; break;
	case NO_DATA:			os << "[Host Error] No Data"; break;
	case NO_RECOVERY:		os << "[Host Error] No Recovery"; break;
	case TRY_AGAIN:			os << "[Host Error] Try Again"; break;
	}
	return os.str();
#endif
}


SocketHandle SocketInterface::create(SocketDomain domain, SocketType socketType,
	SocketProtocol protocol) {
#if defined(_MSC_VER)
	static WSADATA wsaData;
	auto status = WSAStartup(MAKEWORD(2, 2), &wsaData);
	switch(status) {
	case WSASYSNOTREADY:		throw std::runtime_error("The underlying network subsystem is not ready for network communication.");
	case WSAVERNOTSUPPORTED:	throw std::runtime_error("The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation.");
	case WSAEINPROGRESS:		throw std::runtime_error("A blocking Windows Sockets 1.1 operation is in progress.");
	case WSAEPROCLIM:			throw std::runtime_error("A limit on the number of tasks supported by the Windows Sockets implementation has been reached.");
	case WSAEFAULT:				throw std::runtime_error("The lpWSAData parameter is not a valid pointer."); /* unlikely to ever be thrown */
	}
#endif
	SocketHandle handle;
	handle.descriptor = ::socket((int)domain, (int)socketType, (int)protocol);
	ASSERT(
		"SocketInterface::create()\n",
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
		"SocketInterface::setLocalAddressAndPort()\n",
		fillAddress(localAddress, localPort, socketAddress);
	);

	auto status = ::bind(handle.descriptor, (sockaddr*)&socketAddress,
		sizeof(sockaddr_in));

	ASSERT("SocketInterface::setLocalAddressAndPort()\n", status == SOCKET_ERROR);
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


void SocketInterface::fillAddress(const std::string& address,
	unsigned short port, sockaddr_in& socketAddress) {
	memset(&socketAddress, 0, sizeof(socketAddress));
	socketAddress.sin_family = AF_INET; // Internet address
	hostent* host = ::gethostbyname(address.c_str());

	ASSERT("SocketInterface::fillAddress()\n", host == NULL);

	socketAddress.sin_addr.s_addr = *((unsigned long *)host->h_addr_list[0]);
	socketAddress.sin_port = htons(port); // Assign port in network byte order
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
		"SocketInterface::connect()\n",
		fillAddress(address, port, destinationAddress);
	);
	
	auto status = ::connect(handle.descriptor, (sockaddr*)&destinationAddress,
		sizeof(destinationAddress));

	ASSERT("SocketInterface::connect()\n", status == SOCKET_ERROR);
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
		"SocketInterface::sendto()\n",
		fillAddress(address, port, destinationAddress);
	);

	auto status = ::sendto(handle.descriptor, (CCHAR_PTR)buffer, length,
		(int)flags, (sockaddr*)&destinationAddress, sizeof(destinationAddress));

	ASSERT("SocketInterface::sendto()\n", status == SOCKET_ERROR);

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
		os << getErrorMessage();
		throw std::runtime_error(os.str());
	}
#else /* OSX|LINUX */
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
	CATCH_ASSERT(
		"SocketInterface::getNetworkInterfaces()\n",
	#if defined(_MSC_VER)
		return getNetworkInterfaces_Win();
	#else /* OSX|LINUX */
		return getNetworkInterfaces_Nix();	
	#endif
	);
}


std::vector<NetInterface> SocketInterface::getNetworkInterfaces_Win() {
	std::vector<NetInterface> list;
#if defined(_MSC_VER)
	INTERFACE_INFO info[64];
	int length = 0;

	CATCH_ASSERT(
		"SocketInterface::getNetworkInterfaces_Win()\n",
		gniWinNetProbe((void*)&info, sizeof(info), length);
	);

	for(int i = 0; i < length; i++) {
		char buffer[INET_ADDRSTRLEN];
		NetInterface token;
		sockaddr_in address   = info.iiAddress.AddressIn;
		sockaddr_in netmask   = info.iiNetmask.AddressIn;
		sockaddr_in broadcast = info.iiBroadcastAddress.AddressIn;
		token.flags     = (unsigned int)info.iiFlags;

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
	}
#endif

	return list;
}


void SocketInterface::gniWinNetProbe(void* info, int infoLength, int& length) {
#if defined(_MSC_VER)
	DWORD bytesReturned;
	SocketHandle handle;

	CATCH_ASSERT(
		"SocketInterface::gniWinNetProbe()\n",
		handle = create(SocketDomain::INET, SocketType::DATAGRAM,
			SocketProtocol::DEFAULT);
	);

	auto status = WSAIoctl(
		handle.descriptor,
		SIO_GET_INTERFACE_LIST,
		NULL, NULL,
		info, infoLength,
		&bytesReturned,
		NULL, NULL
	);
	length = bytesReturned / sizeof(INTERFACE_INFO);

	std::ostringstream os;
	if(status == SOCKET_ERROR) {
		os << "SocketInterface::gniWinNetProbe()\n");
		os << getErrorMessage() << std::endl;
		// don't throw yet until socket is closed
	}

	try { close(); }
	catch(std::runtime_error e) {
		os << e.what() << std::endl;
		throw std::runtime_error(os.str());
	}
	if(status == SOCKET_ERROR) throw std::runtime_error(os.str());
#else
	UNUSED(info);
	UNUSED(infoLength);
	UNUSED(length);
#endif
}


std::vector<NetInterface> SocketInterface::getNetworkInterfaces_Nix() {
	std::vector<NetInterface> list;

#ifndef _MSC_VER

#endif

	return list;
}