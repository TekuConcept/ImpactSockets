/**
 *	Created by TekuConcept on May 12, 2017
 */

#include "TcpServer.h"

using namespace Impact;

TcpServer::TcpServer(unsigned short port) : server(port) {}



TcpServer::~TcpServer() {}



TcpSocPtr TcpServer::accept() {
    std::shared_ptr<TCPSocket> socket(server.accept());
    TcpSocPtr connection = std::make_shared<TcpClient>();
    connection->socket = socket;
    connection->connected = true;
    connection->pollToken.add(&socket->getHandle(), POLLIN | POLLHUP);
    return connection;
}



int TcpServer::getPort() {
    return server.getLocalPort();
}



SocketHandle& TcpServer::getHandle() {
    return server.getHandle();
}



int TcpServer::waitForClient(int timeout) {
    SocketHandle* handles[] = {&server.getHandle()};
    return Socket::select(handles, 1, timeout);
}
