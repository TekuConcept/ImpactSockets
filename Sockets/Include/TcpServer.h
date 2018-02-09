/**
*	Created by TekuConcept on May 12, 2017
*/

#ifndef _TCP_SERVER_
#define _TCP_SERVER_

#include "CrossPlatformSockets.h"
#include "Sockets.h"
#include "TcpClient.h"

namespace Impact {
	class TcpServer;
	typedef std::shared_ptr<TcpClient> TcpSocPtr;
	
	class TcpServer {
	private:
		TCPServerSocket server;

	public:
		API_DECLSPEC TcpServer(unsigned short port);
		TcpSocPtr API_DECLSPEC accept();
		virtual API_DECLSPEC ~TcpServer();
		int API_DECLSPEC getPort();
		int API_DECLSPEC waitForClient(int timeout = -1);
	};
}

#endif