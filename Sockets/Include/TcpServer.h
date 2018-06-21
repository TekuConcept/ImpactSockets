/**
*	Created by TekuConcept on May 12, 2017
*/

#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

// #include "CrossPlatformSockets.h"
// #include "Sockets.h"
// #include "TcpClient.h"
#include "SocketHandle.h"
#include "TcpSocket.h"
#include <string>
#include <functional>

#define DEFAULT_LENGTH 5

namespace Impact {
	class TcpServer {
		SocketHandle _handle_;

		bool _isOpen_;

		void initialize();
		void open(std::function<void ()>, int backlog);

	public:
		TcpServer();
		TcpServer(unsigned short port, int backlog = DEFAULT_LENGTH)
			/* throw(std::runtime_error) */;
		TcpServer(unsigned short port, const std::string& address,
			int backlog = DEFAULT_LENGTH)
			/* throw(std::runtime_error) */;
		virtual ~TcpServer();

		virtual void open(unsigned short port, int backlog = DEFAULT_LENGTH)
			/* throw(std::runtime_error) */;
		virtual void open(unsigned short port, const std::string& address,
			int backlog = DEFAULT_LENGTH)
			/* throw(std::runtime_error) */;
		bool is_open() const;
		virtual void close()
			/* throw(std::runtime_error) */;

		virtual void accept(TcpSocket& socket)
			/* throw(std::runtime_error) */;
		int port()
			/* throw(std::runtime_error) */;
	};
}

#undef DEFAULT_LENGTH
#endif