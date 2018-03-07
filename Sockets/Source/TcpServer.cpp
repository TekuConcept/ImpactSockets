/**
*  Created by TekuConcept on May 12, 2017
*/

#include "TcpServer.h"

using namespace Impact;

TcpServer::TcpServer(unsigned short port) : server(port) {}



TcpServer::~TcpServer() {}



TcpServer::TcpSocPtr TcpServer::accept() {
	std::shared_ptr<TCPSocket> socket(server.accept());
	TcpSocPtr connection = std::make_shared<TcpClient>();
	connection->socket = socket;
	connection->pollToken.add(socket->getHandle(), POLLIN);
	connection->connected = true;
	return connection;
}



int TcpServer::getPort() {
	return server.getLocalPort();
}



SocketHandle& TcpServer::getHandle() {
	return server.getHandle();
}