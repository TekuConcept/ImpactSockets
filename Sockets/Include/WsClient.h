/**
 * Created by TekuConcept on July 21, 2017
 */

#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#include "CrossPlatformSockets.h"
#include "Sockets.h"
#include <string>
#include <memory>

namespace Impact {
	class WsClient {
	public:
		API_DECLSPEC WsClient();
		API_DECLSPEC WsClient(int port, std::string address = "127.0.0.1");
		API_DECLSPEC ~WsClient();

		int API_DECLSPEC connect(int port, std::string address);
		void API_DECLSPEC disconnect();
		bool API_DECLSPEC isConnected();
		
		void API_DECLSPEC write(std::string text);
		char API_DECLSPEC read();

	private:
		std::shared_ptr<TCPSocket> socket;
		bool connected;
	};
}

#endif
