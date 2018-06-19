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

		static std::string getHostErrorMessage();
		static void fillAddress(const std::string&, unsigned short port,
			sockaddr_in&);

	public:
		/*                    *\
		| GENERIC SOCKET INFO  |
		\*                    */
		static std::string getErrorMessage();
		static std::string getLocalAddress(const SocketHandle& handle)
			/* throw(std::runtime_error) */;
		static unsigned short getLocalPort(const SocketHandle& handle)
			/* throw(std::runtime_error) */;
		static void setLocalPort(const SocketHandle& handle, unsigned short localPort)
			/* throw(std::runtime_error) */;
		static void setLocalAddressAndPort(const SocketHandle& handle,
			const std::string& localAddress, unsigned short localPort = 0)
			/* throw(std::runtime_error) */;
		static unsigned short resolveService(const std::string& service,
			const std::string& protocol = "tcp");
		std::string getForeignAddress(const SocketHandle& handle)
			/* throw(std::runtime_error) */;
		unsigned short getForeignPort(const SocketHandle& handle)
			/* throw(std::runtime_error) */;

		/*                    *\
		| COMMUNICATION        |
		\*                    */
		void connect(const SocketHandle& handle,
			const std::string& foreignAddress, unsigned short foreignPort)
			/* throw(std::runtime_error) */;
		void shutdown(const SocketHandle& handle)
			/* throw(std::runtime_error) */;
		void send(const SocketHandle& handle, const void* buffer, int bufferLen,
			MessageFlags flags = MessageFlags::NONE)
			/* throw(std::runtime_error) */;
		int recv(const SocketHandle& handle, void* buffer, int bufferLen,
			MessageFlags flags = MessageFlags::NONE)
			/* throw(std::runtime_error) */;
		// poll
		// select
		// keepalive
	};
}

#endif
