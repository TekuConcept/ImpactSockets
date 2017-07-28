/**
 * Created by TekuConcept on July 21, 2017
 */

#ifndef WEB_CLIENT_H
#define WEB_CLIENT_H

#include "CrossPlatformSockets.h"
#include "Sockets.h"
#include <string>
#include <memory>

namespace Impact {
	class WebClient {
	public:
		API_DECLSPEC WebClient();
		API_DECLSPEC WebClient(int port, std::string address = "127.0.0.1");
		API_DECLSPEC ~WebClient();

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
