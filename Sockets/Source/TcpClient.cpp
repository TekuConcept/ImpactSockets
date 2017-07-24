/**
 *  Created by TekuConcept on Mar 29, 2016
 */

#include "TcpClient.h"
#include <iostream>

using namespace Impact;

TcpClient::TcpClient() :
    std::iostream(this),
    outputBuffer_(new char[BUF_SIZE + 1]),
    inputBuffer_(new char[BUF_SIZE + 1]),
    self(typeid(this), this) {
    init();
}



TcpClient::TcpClient(int port, std::string address) :
    std::iostream(this),
    outputBuffer_(new char[BUF_SIZE + 1]),
    inputBuffer_(new char[BUF_SIZE + 1]),
    self(typeid(this), this) {
    init();
    connect(port, address);
}



TcpClient::~TcpClient() {}



void TcpClient::init() {
    connected = false;
    peerConnected = false;
    
    setp(outputBuffer_, outputBuffer_ + BUF_SIZE - 1);
    setg(inputBuffer_, inputBuffer_ + BUF_SIZE - 1, inputBuffer_ + BUF_SIZE - 1);
}



short TcpClient::checkFlags(short events) {
    // set internal flags to handle user actions in advance
    if((events & POLLHUP) != 0)
        peerConnected = false;
    // handle POLLERR?
    return events;
}



int TcpClient::connect(int port, std::string address) {
    if(connected) return 2;
    try {
        socket = std::make_shared<TCPSocket>(address, port);
        socket->setEvents(POLLIN);
        connected = true;
        peerConnected = true;
    }
    catch (SocketException &e) {
        std::cerr << "TcpClient: " << e.what() << std::endl;
        connected = false;
        peerConnected = false;
        return 1;
    }
    return 0;
}



void TcpClient::disconnect() {
    if (socket != nullptr && connected) {
        if(peerConnected) {
            try {
                socket->disconnect();
            } catch(SocketException) {
                // server disconnected first,
                // ignore for now
                std::cerr << "Peer disconnected first" << std::endl;
            }
            peerConnected = false;
        }
        connected = false;
    }
}



bool TcpClient::isConnected() {
    return connected;
}



void TcpClient::setTimeout(int time_ms) {
	// -1 means waiting indefinitely ie no timeout
	if (time_ms < -1) timeout = -1; // normalize
	else              timeout = time_ms;
}



int TcpClient::sync() {
    int len = int(pptr() - pbase());
    if(socket != nullptr && connected)
        socket->send(pbase(), len);
    setp(pbase(), epptr());
    return 0;
}



int TcpClient::underflow() {
    if(socket != nullptr && connected) {
        short isr;
        if(socket->poll(isr, timeout) == 0) {
            EventArgs e;
            onTimeout.invoke(self, e);
        }
        else if((checkFlags(isr) & POLLIN) > 0) {
            int bytesReceived = socket->recv(eback(), BUF_SIZE);
            setg(eback(), eback(), eback() + bytesReceived);
            return *eback();
        }
    }
    return EOF;
}