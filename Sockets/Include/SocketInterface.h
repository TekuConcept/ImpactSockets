/**
 * Created by TekuConcept on June 15, 2018
 */

#ifndef _SOCKET_INTERFACE_H_
#define _SOCKET_INTERFACE_H_

#include "Environment.h"
#include "SocketHandle.h"
#include "SocketPollTable.h"
#include <string>            // For string
#include <vector>

namespace Impact {
	class SocketInterface {
		SocketInterface(); // static-only class

	public:
		/*                    *\
		| SOCKET OBJECT        |
		\*                    */
		static SocketHandle create(SocketDomain domain,
			SocketType socketType, SocketProtocol protocol)
			/* throw(std::runtime_error) */;

		static void close(SocketHandle& handle)
			/* throw(std::runtime_error) */;


		/*                    *\
		| GENERIC SOCKET INFO  |
		\*                    */
		static std::string getLocalAddress(const SocketHandle& handle)
			/* throw(std::runtime_error) */;

		static unsigned short getLocalPort(const SocketHandle& handle)
			/* throw(std::runtime_error) */;

		static void setLocalPort(const SocketHandle& handle,
			unsigned short localPort)
			/* throw(std::runtime_error) */;

		static void setLocalAddressAndPort(const SocketHandle& handle,
			const std::string& localAddress, unsigned short localPort = 0)
			/* throw(std::runtime_error) */;

		static void setBroadcast(const SocketHandle& handle, bool enabled)
			/* throw(std::runtime_error) */;

		static void setMulticastTTL(const SocketHandle& handle,
			unsigned char timeToLive = 1)
			/* throw(std::runtime_error) */;

		static unsigned short resolveService(const std::string& service,
			const std::string& protocol = "tcp");

		static std::string getForeignAddress(const SocketHandle& handle)
			/* throw(std::runtime_error) */;

		static unsigned short getForeignPort(const SocketHandle& handle)
			/* throw(std::runtime_error) */;


		/*                    *\
		| COMMUNICATION        |
		\*                    */
		static void connect(const SocketHandle& handle,
			unsigned short port, const std::string& address)
			/* throw(std::runtime_error) */;

		static void listen(const SocketHandle& handle, int backlog = 5)
			/* throw(std::runtime_error) */;

		static void shutdown(const SocketHandle& handle,
			SocketChannel channel = SocketChannel::BOTH)
			/* throw(std::runtime_error) */;

		static void accept(const SocketHandle& handle, SocketHandle& peer)
			/* throw(std::runtime_error) */;

		static void group(const SocketHandle& handle, std::string multicastName,
			GroupApplication method)
			/* throw(std::runtime_error) */;

		static void send(const SocketHandle& handle, const void* buffer,
			int length, MessageFlags flags = MessageFlags::NONE)
			/* throw(std::runtime_error) */;

		static int sendto(const SocketHandle& handle, const void* buffer,
			int length, unsigned short port, const std::string& address,
			MessageFlags flags = MessageFlags::NONE);
			/* throw(std::runtime_error) */;

		static int recv(const SocketHandle& handle, void* buffer, int length,
			MessageFlags flags = MessageFlags::NONE)
			/* throw(std::runtime_error) */;

		static int recvfrom(const SocketHandle& handle, void* buffer,
			int length, unsigned short& port, std::string& address,
			MessageFlags flags = MessageFlags::NONE)
			/* throw(std::runtime_error) */;

		static void keepalive(const SocketHandle& handle,
			KeepAliveOptions options)
			/* throw(std::runtime_error) */;

		static int select(
			std::vector<SocketHandle*> readHandles,
			std::vector<SocketHandle*> writeHandles,
			int timeout=-1, unsigned int microTimeout=0)
			/* throw(std::runtime_error) */;

		static int poll(SocketPollTable& token, int timeout)
			/* throw(std::runtime_error) */;
	};
}

#endif
