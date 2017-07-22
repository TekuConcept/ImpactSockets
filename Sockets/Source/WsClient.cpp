/**
 * Created by TekuConcept on July 21, 2017
 */

#include "WsClient.h"
#include <sstream>

using namespace Impact;



WsClient::WsClient() : connected(false) {}



WsClient::WsClient(int port, std::string address)
	: connected(false) {
	connect(port, address);
}



WsClient::~WsClient() {}



int WsClient::connect(int port, std::string address) {
	// open a socket connection
	if (connected) return -2;
	try {
		socket = std::make_shared<TCPSocket>(address, port);
		connected = true;
	}
	catch (SocketException &e) {
		std::cerr << "WsClient: " << e.what() << std::endl;
		connected = false;
		return -1;
	}
	
	// send handshake message
	// listen for server's handshake
	return 0;
}



void WsClient::disconnect() {
	if (socket != nullptr && connected) {
		socket->disconnect();
		connected = false;
	}
}



bool WsClient::isConnected() {
	return connected;
}



void WsClient::write(std::string text) {
	if (socket != nullptr && connected)
		socket->send(text.c_str(), text.length());
}

char WsClient::read() {
	char c[1] = { '\0' };
	if (socket != nullptr && connected)
		socket->recv(c, 1);
	return c[0];
}