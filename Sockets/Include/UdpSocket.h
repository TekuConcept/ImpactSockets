/**
 * Created by TekuConcept on June 21, 2018
 */

#ifndef _UDP_SOCKET_H_
#define _UDP_SOCKET_H_

#include "SocketHandle.h"
#include <string>
#include <functional>

namespace Impact {
	class UdpSocket {
		SocketHandle _handle_;

		bool _isOpen_;

		void initialize();
		void setBroadcast();
		void open(std::function<void ()>);

	public:
		UdpSocket();
		UdpSocket(unsigned short port)
			/* throw(std::runtime_error) */;
		UdpSocket(unsigned short port, const std::string& address)
			/* throw(std::runtime_error) */;
		virtual ~UdpSocket();

		virtual void open()
			/* throw(std::runtime_error) */;
		virtual void open(unsigned short port)
			/* throw(std::runtime_error) */;
		virtual void open(unsigned short port, const std::string& address);
			/* throw(std::runtime_error) */;
		bool is_open() const;
		virtual void close()
			/* throw(std::runtime_error) */;

		int sendTo(const void* buffer, int length,
			unsigned short targetPort, const std::string& targetAddress);
		int recvFrom(void* buffer, int length,
			unsigned short& sourcePort, std::string& sourceAddress);

		// void setMulticastTTL(unsigned char multicastTTL);
		// void joinGroup(const std::string& multicastGroup);
		// void leaveGroup(const std::string& multicastGroup);
	};
}

#endif