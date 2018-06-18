/**
 * Created by TekuConcept on June 15, 2018
 */

#ifndef _SOCKET_INTERFACE_H_
#define _SOCKET_INTERFACE_H_

#include <iostream>
#include <string>            // For string
#include <cstring>           // For strerror, atoi, and memset
#include <exception>         // For exception class

#include "SocketHandle.h"

#if defined(_MSC_VER)
	#include <winsock2.h>
#else
	#include <sys/poll.h>    // For struct pollfd, poll()
	#include <netinet/in.h>  // For sockaddr_in
#endif

namespace Impact {
	class SocketInterface {
		SocketInterface(); // static-only class

		static std::string getErrorMessage();
		static std::string getHostErrorMessage();
		static void fillAddress(const std::string&, unsigned short port,
			sockaddr_in&);

	public:
		static std::string getLocalAddress(SocketHandle handle)
			/* throw(std::runtime_error) */;
		static unsigned short getLocalPort(SocketHandle handle)
			/* throw(std::runtime_error) */;
		static void setLocalPort(SocketHandle handle, unsigned short localPort)
			/* throw(std::runtime_error) */;
		static void setLocalAddressAndPort(SocketHandle handle,
			const std::string& localAddress, unsigned short localPort = 0)
			/* throw(std::runtime_error) */;
		static unsigned short resolveService(const std::string& service,
			const std::string& protocol = "tcp");
	};
}

#endif
