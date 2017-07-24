/**
 *	Created by TekuConcept on May 12, 2017
 */

#include "TcpServer.h"

using namespace Impact;

TcpServer::TcpServer(unsigned short port) : server(port) {}



TcpServer::~TcpServer() {}


#include <iostream>
TcpServer::TcpSocPtr TcpServer::accept() {
    std::shared_ptr<TCPSocket> socket(server.accept());
    TcpSocPtr connection = std::make_shared<TcpClient>();
    socket->setEvents(POLLIN);
    connection->socket = socket;
    connection->connected = true;
    connection->peerConnected = true;
	return connection;
}

int TcpServer::getPort() {
    return server.getLocalPort();
}