/**
*	Created by TekuConcept on Mar 29, 2016
*/

#ifndef _TCP_CLIENT_
#define _TCP_CLIENT_

#include "CrossPlatformSockets.h"
#include "Sockets.h"
#include <streambuf>
#include <stdio.h>
#include <string>
#include <memory>

#include "EventHandler.h"

namespace Impact {
	class TcpServer;
	class TcpClient : private std::streambuf, public std::iostream {
		friend TcpServer;

	public:
		API_DECLSPEC TcpClient();
		API_DECLSPEC TcpClient(int port, std::string address = "127.0.0.1");
		virtual API_DECLSPEC ~TcpClient();
		
		int API_DECLSPEC connect(int port, std::string address);
		void API_DECLSPEC disconnect();
		bool API_DECLSPEC isConnected();
		
		int API_DECLSPEC sync();
		int API_DECLSPEC underflow();
		
		void API_DECLSPEC setTimeout(int time_ms);
		EventHandler<EventArgs> onTimeout;
		
		SocketHandle& getHandle();

	private:
		static const unsigned int BUF_SIZE = 256;
		char* outputBuffer_;
		char* inputBuffer_;
		std::shared_ptr<TCPSocket> socket;
		SocketPollToken pollToken;
		bool connected;
		int timeout_;
		Object self;

		void init();
		void checkFlags();
	};
}

#endif
