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

namespace Impact {
	class TcpServer;
	class TcpClient : private std::streambuf, public std::iostream
	{
	public:
		friend TcpServer;
		API_DECLSPEC TcpClient();
		API_DECLSPEC TcpClient(int port, std::string address = "127.0.0.1");
		int API_DECLSPEC connect(int port, std::string address);
		void API_DECLSPEC disconnect();
		int API_DECLSPEC sync();
		int API_DECLSPEC underflow();
		bool API_DECLSPEC isConnected();
		virtual API_DECLSPEC ~TcpClient();

	private:
		static const unsigned int BUF_SIZE = 256;
		char* outputBuffer_;
		char* inputBuffer_;
		std::shared_ptr<TCPSocket> socket;
		bool connected;

		void init_();
	};
}

#endif
