/*
*   C++ sockets on Unix and Windows
*   Copyright (C) 2002
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*   Modified by TekuConcept on May 12, 2017, for Windows support.
*   Modified by TekuConcept on July 18, 2017, for Mac support.
*   Modified by TekuConcept on July 23, 2017, for poll feature.
*/

#include "Sockets.h"

#include <sys/types.h>       // For data types

#if defined(_MSC_VER)
  #include <ws2tcpip.h>
  #define SOC_POLL WSAPoll
#else
  #include <sys/socket.h>      // For socket(), connect(), send(), and recv()
  #include <netdb.h>           // For gethostbyname()
  #include <arpa/inet.h>       // For inet_addr()
  #include <unistd.h>          // For close()
  #include <netinet/in.h>      // For sockaddr_in
  #define SOC_POLL ::poll
#endif

#include <cstring>           // For strerror and memset
#include <stdlib.h>          // For atoi
typedef void raw_type;       // Type used for raw data on this platform
#include <errno.h>           // For errno

#if defined(_MSC_VER)
	#define CLOSE_SOCKET(x) closesocket(x)
	#define SOC_SD_HOW SD_BOTH
	#define CCHAR_PTR const char *
	#define CHAR_PTR char *

	#pragma warning(disable:4996)
	#pragma comment (lib, "Ws2_32.lib")
	#pragma comment (lib, "Mswsock.lib")
	#pragma comment (lib, "AdvApi32.lib")
#else
	#define CLOSE_SOCKET(x) ::close(x)
	#define SOC_SD_HOW SHUT_RDWR
	#define CCHAR_PTR raw_type *
	#define CHAR_PTR raw_type *
#endif

#define string std::string
#define exception std::exception
using namespace Impact;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
//                            SocketException Code                           //
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

SocketException::SocketException(const string &message, bool inclSysMsg)
throw() : userMessage(message) {
	if (inclSysMsg) {
		userMessage.append(": ");
		userMessage.append(strerror(errno));
	}
}



SocketException::~SocketException() throw() {
}



const char *SocketException::what() const throw() {
	return userMessage.c_str();
}



// Function to fill in address structure given an address and port
static void fillAddr(const string &address, unsigned short port,
	sockaddr_in &addr) {
	memset(&addr, 0, sizeof(addr));  // Zero out address structure
	addr.sin_family = AF_INET;       // Internet address

	hostent *host;  // Resolve name
	if ((host = gethostbyname(address.c_str())) == NULL) {
		// strerror() will not work for gethostbyname() and hstrerror()
		// is supposedly obsolete
		throw SocketException("Failed to resolve name (gethostbyname())");
	}
	addr.sin_addr.s_addr = *((unsigned long *)host->h_addr_list[0]);

	addr.sin_port = htons(port);     // Assign port in network byte order
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
//                                Socket Code                                //
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

Socket::Socket(int type, int protocol) throw(SOC_EXCEPTION) {

#if defined(_MSC_VER)
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		throw SocketException("Windows Socket Startup Failed", true);
	}
#endif
	// Make a new socket
	if ((descriptor = socket(PF_INET, type, protocol)) < 0) {
		throw SocketException("Socket creation failed (socket())", true);
	}
}



Socket::Socket(int socketDescriptor) {
	descriptor = socketDescriptor;
}



Socket::~Socket() {
	CLOSE_SOCKET(descriptor);
	descriptor = -1;

#if defined(_MSC_VER)
	WSACleanup();
#endif
}



SocketHandle& Socket::getHandle() {
	return *this;
}



string Socket::getLocalAddress() throw(SOC_EXCEPTION) {
	sockaddr_in addr;
	unsigned int addr_len = sizeof(addr);

	if (getsockname(descriptor, (sockaddr *)&addr, (socklen_t *)&addr_len) < 0) {
		throw SocketException("Fetch of local address failed (getsockname())", true);
	}
	return inet_ntoa(addr.sin_addr);
}



unsigned short Socket::getLocalPort() throw(SOC_EXCEPTION) {
	sockaddr_in addr;
	unsigned int addr_len = sizeof(addr);

	if (getsockname(descriptor, (sockaddr *)&addr, (socklen_t *)&addr_len) < 0) {
		throw SocketException("Fetch of local port failed (getsockname())", true);
	}
	return ntohs(addr.sin_port);
}



void Socket::setLocalPort(unsigned short localPort) throw(SOC_EXCEPTION) {
	// Bind the socket to its port
	sockaddr_in localAddr;
	memset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddr.sin_port = htons(localPort);

	if (::bind(descriptor, (sockaddr *)&localAddr, sizeof(sockaddr_in)) < 0) {
		throw SocketException("Set of local port failed (bind())", true);
	}
}



void Socket::setLocalAddressAndPort(const string &localAddress,
	unsigned short localPort) throw(SOC_EXCEPTION) {
	// Get the address of the requested host
	sockaddr_in localAddr;
	fillAddr(localAddress, localPort, localAddr);

	if (::bind(descriptor, (sockaddr *)&localAddr, sizeof(sockaddr_in)) < 0) {
		throw SocketException("Set of local address and port failed (bind())", true);
	}
}



unsigned short Socket::resolveService(const string &service,
	const string &protocol) {
	struct servent *serv;        /* Structure containing service information */

	if ((serv = getservbyname(service.c_str(), protocol.c_str())) == NULL)
		return static_cast<unsigned short>(atoi(service.c_str()));  /* Service is port number */
	else
		return ntohs(serv->s_port);    /* Found port (network byte order) by name */
}



int Socket::select(SocketHandle** handles, int length, struct timeval* timeout) {
	fd_set set;
	int nfds = 0, rval;
	
	FD_ZERO(&set);
	for(int i = 0; i < length; i++) {
		auto handle = handles[i]->descriptor;
		FD_SET(handle, &set);
		if(handle > nfds) nfds = handle;
	}
	
	rval = ::select(nfds + 1, &set, NULL, NULL, timeout);
	return rval;
}



int Socket::select(SocketHandle** handles, int length, unsigned int timeout) {
	struct timeval time_s;
	time_s.tv_sec = timeout;
	time_s.tv_usec = 0;
	return Socket::select(handles, length, &time_s);
}



int Socket::poll(SocketPollToken* handles, int length, int timeout) {
	int rtn;
	struct pollfd* fds = new pollfd[length];
	for(int i = 0; i < length; i++) {
		fds[i].fd = handles[i].handle->descriptor;
		fds[i].events = handles[i].events;
		fds[i].revents = 0;
	}
	rtn = SOC_POLL(fds, length, timeout);
	for(int i = 0; i < length; i++)
		handles[i].revents = fds[i].revents;
	delete[] fds;
	return rtn;
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
//                        CommunicatingSocket Code                           //
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

CommunicatingSocket::CommunicatingSocket(int type, int protocol)
throw(SOC_EXCEPTION) : Socket(type, protocol) {}



CommunicatingSocket::CommunicatingSocket(int newConnSD) : Socket(newConnSD) {}



void CommunicatingSocket::connect(const string &foreignAddress,
	unsigned short foreignPort) throw(SOC_EXCEPTION) {
	// Get the address of the requested host
	sockaddr_in destAddr;
	fillAddr(foreignAddress, foreignPort, destAddr);

	// Try to connect to the given port
	if (::connect(descriptor, (sockaddr *)&destAddr, sizeof(destAddr)) < 0) {
		throw SocketException("Connect failed (connect())", true);
	}
}



void CommunicatingSocket::disconnect()
throw(SOC_EXCEPTION) {
	if (shutdown(descriptor, SOC_SD_HOW) < 0) {
		throw SocketException("Shutdown failed (disconnect())", true);
	}
}



void CommunicatingSocket::send(const void *buffer, int bufferLen)
throw(SOC_EXCEPTION) {
	if (::send(descriptor, (CCHAR_PTR)buffer, bufferLen, 0) < 0) {
		throw SocketException("Send failed (send())", true);
	}
}



int CommunicatingSocket::recv(void *buffer, int bufferLen)
throw(SOC_EXCEPTION) {
	int rtn;
	if ((rtn = ::recv(descriptor, (CHAR_PTR)buffer, bufferLen, 0)) < 0) {
		throw SocketException("Received failed (recv())", true);
	}

	return rtn;
}



// void CommunicatingSocket::setEvents(short events) {
// 	fds[0].events = events;
// }



// int CommunicatingSocket::poll(short &revents, int timeout)
// 	throw(SOC_EXCEPTION) {
// 	int rtn;
// 	fds[0].revents = 0;
// 	if((rtn = SOC_POLL(fds, 1, timeout)) < 0) {
// 		throw SocketException("Poll failed (poll())", true);
// 	}
// 	revents = fds[0].revents;
// 	return rtn;
// }



string CommunicatingSocket::getForeignAddress()
throw(SOC_EXCEPTION) {
	sockaddr_in addr;
	unsigned int addr_len = sizeof(addr);

	if (getpeername(descriptor, (sockaddr *)&addr, (socklen_t *)&addr_len) < 0) {
		throw SocketException("Fetch of foreign address failed (getpeername())", true);
	}
	return inet_ntoa(addr.sin_addr);
}



unsigned short CommunicatingSocket::getForeignPort() throw(SOC_EXCEPTION) {
	sockaddr_in addr;
	unsigned int addr_len = sizeof(addr);

	if (getpeername(descriptor, (sockaddr *)&addr, (socklen_t *)&addr_len) < 0) {
		throw SocketException("Fetch of foreign port failed (getpeername())", true);
	}
	return ntohs(addr.sin_port);
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
//                               TCPSocket Code                              //
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

TCPSocket::TCPSocket()
throw(SOC_EXCEPTION) : CommunicatingSocket(SOCK_STREAM, IPPROTO_TCP) {}



TCPSocket::TCPSocket(const string &foreignAddress, unsigned short foreignPort)
throw(SOC_EXCEPTION) : CommunicatingSocket(SOCK_STREAM, IPPROTO_TCP) {
	connect(foreignAddress, foreignPort);
}



TCPSocket::TCPSocket(int newConnSD) : CommunicatingSocket(newConnSD) {}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
//                            TCPServerSocket Code                           //
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

TCPServerSocket::TCPServerSocket(unsigned short localPort, int queueLen)
throw(SOC_EXCEPTION) : Socket(SOCK_STREAM, IPPROTO_TCP) {
	setLocalPort(localPort);
	setListen(queueLen);
}



TCPServerSocket::TCPServerSocket(const string &localAddress,
	unsigned short localPort, int queueLen)
	throw(SOC_EXCEPTION) : Socket(SOCK_STREAM, IPPROTO_TCP) {
	setLocalAddressAndPort(localAddress, localPort);
	setListen(queueLen);
}



TCPSocket *TCPServerSocket::accept() throw(SOC_EXCEPTION) {
	int newConnSD;
	if ((newConnSD = ::accept(descriptor, NULL, 0)) < 0) {
		throw SocketException("Accept failed (accept())", true);
	}

	return new TCPSocket(newConnSD);
}



void TCPServerSocket::setListen(int queueLen) throw(SOC_EXCEPTION) {
	if (listen(descriptor, queueLen) < 0) {
		throw SocketException("Set listening socket failed (listen())", true);
	}
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
//                               UDPSocket Code                              //
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

UDPSocket::UDPSocket() throw(SOC_EXCEPTION) : CommunicatingSocket(SOCK_DGRAM,
	IPPROTO_UDP) {
	setBroadcast();
}



UDPSocket::UDPSocket(unsigned short localPort)  throw(SOC_EXCEPTION) :
	CommunicatingSocket(SOCK_DGRAM, IPPROTO_UDP) {
	setLocalPort(localPort);
	setBroadcast();
}



UDPSocket::UDPSocket(const string &localAddress, unsigned short localPort)
throw(SOC_EXCEPTION) : CommunicatingSocket(SOCK_DGRAM, IPPROTO_UDP) {
	setLocalAddressAndPort(localAddress, localPort);
	setBroadcast();
}



void UDPSocket::setBroadcast() {
	// If this fails, we'll hear about it when we try to send.  This will allow
	// system that cannot broadcast to continue if they don't plan to broadcast
	int broadcastPermission = 1;
	setsockopt(descriptor, SOL_SOCKET, SO_BROADCAST,
		(CCHAR_PTR)&broadcastPermission, sizeof(broadcastPermission));
}



void UDPSocket::disconnect() throw(SOC_EXCEPTION) {
	sockaddr_in nullAddr;
	memset(&nullAddr, 0, sizeof(nullAddr));
	nullAddr.sin_family = AF_UNSPEC;

	// Try to disconnect
	if (::connect(descriptor, (sockaddr *)&nullAddr, sizeof(nullAddr)) < 0) {
		if (errno != EAFNOSUPPORT) {
			throw SocketException("Disconnect failed (connect())", true);
		}
	}
}



void UDPSocket::sendTo(const void *buffer, int bufferLen,
	const string &foreignAddress, unsigned short foreignPort)
	throw(SOC_EXCEPTION) {
	sockaddr_in destAddr;
	fillAddr(foreignAddress, foreignPort, destAddr);

	// Write out the whole buffer as a single message.
	if (sendto(descriptor, (CCHAR_PTR)buffer, bufferLen, 0,
		(sockaddr *)&destAddr, sizeof(destAddr)) != bufferLen) {
		throw SocketException("Send failed (sendto())", true);
	}
}



int UDPSocket::recvFrom(void *buffer, int bufferLen, string &sourceAddress,
	unsigned short &sourcePort) throw(SOC_EXCEPTION) {
	sockaddr_in clntAddr;
	socklen_t addrLen = sizeof(clntAddr);
	int rtn;
	if ((rtn = recvfrom(descriptor, (CHAR_PTR)buffer, bufferLen, 0,
		(sockaddr *)&clntAddr, (socklen_t *)&addrLen)) < 0) {
		throw SocketException("Receive failed (recvfrom())", true);
	}
	sourceAddress = inet_ntoa(clntAddr.sin_addr);
	sourcePort = ntohs(clntAddr.sin_port);

	return rtn;
}



void UDPSocket::setMulticastTTL(unsigned char multicastTTL) throw(SOC_EXCEPTION) {
	if (setsockopt(descriptor, IPPROTO_IP, IP_MULTICAST_TTL,
		(CCHAR_PTR)&multicastTTL, sizeof(multicastTTL)) < 0) {
		throw SocketException("Multicast TTL set failed (setsockopt())", true);
	}
}



void UDPSocket::joinGroup(const string &multicastGroup) throw(SOC_EXCEPTION) {
	struct ip_mreq multicastRequest;

	multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastGroup.c_str());
	multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(descriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(CCHAR_PTR)&multicastRequest,
		sizeof(multicastRequest)) < 0) {
		throw SocketException("Multicast group join failed (setsockopt())", true);
	}
}



void UDPSocket::leaveGroup(const string &multicastGroup) throw(SOC_EXCEPTION) {
	struct ip_mreq multicastRequest;

	multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastGroup.c_str());
	multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(descriptor, IPPROTO_IP, IP_DROP_MEMBERSHIP,
		(CCHAR_PTR)&multicastRequest,
		sizeof(multicastRequest)) < 0) {
		throw SocketException("Multicast group leave failed (setsockopt())", true);
	}
}
