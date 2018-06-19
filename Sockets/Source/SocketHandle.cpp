/**
 * Created by TekuConcept on June 15, 2018
 */

#include "SocketHandle.h"
#include "SocketInterface.h"
#include <stdexcept>
#include <string>

#if defined(_MSC_VER)
 	#include <winsock2.h>
#else
	#include <unistd.h> // For close()
#endif

using namespace Impact;

#if defined(_MSC_VER)
	#define CLOSE_SOCKET(x) closesocket(x)
#else
	#define CLOSE_SOCKET(x) ::close(x)
 	#define INVALID_SOCKET -1
#endif

SocketHandle::SocketHandle(int handle) : descriptor(handle) {}


SocketHandle::SocketHandle(SocketType socketType, SocketProtocol protocol,
	SocketDomain domain) :
	descriptor(INVALID_SOCKET) {
#if defined(_MSC_VER)
	static WSADATA wsaData;
	auto status = WSAStartup(MAKEWORD(2, 2), &wsaData);
	switch(status) {
	case WSASYSNOTREADY:     throw std::runtime_error("The underlying network subsystem is not ready for network communication.");
	case WSAVERNOTSUPPORTED: throw std::runtime_error("The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation.");
	case WSAEINPROGRESS:     throw std::runtime_error("A blocking Windows Sockets 1.1 operation is in progress.");
	case WSAEPROCLIM:        throw std::runtime_error("A limit on the number of tasks supported by the Windows Sockets implementation has been reached.");
	case WSAEFAULT:          throw std::runtime_error("The lpWSAData parameter is not a valid pointer."); /* unlikely to ever be thrown */
	}
#endif
	descriptor = ::socket((int)domain, (int)socketType, (int)protocol);
	if (descriptor == INVALID_SOCKET) {
		std::string message("SocketHandle::SocketHandle() ");
		message.append(SocketInterface::getErrorMessage());
		throw std::runtime_error(message);
	}
}


SocketHandle::~SocketHandle() {
	if(descriptor != INVALID_SOCKET) {
		CLOSE_SOCKET(descriptor);
		descriptor = INVALID_SOCKET;
	}
#if defined(_MSC_VER)
	WSACleanup();
#endif
}