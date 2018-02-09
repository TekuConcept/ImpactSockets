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
    
    setp(outputBuffer_, outputBuffer_ + BUF_SIZE - 1);
    setg(inputBuffer_, inputBuffer_ + BUF_SIZE - 1, inputBuffer_ + BUF_SIZE - 1);
}



short TcpClient::checkFlags(short events) {
    // set internal flags to handle user actions in advance
    if((events & POLLHUP) != 0)
        connected = false;
    // handle POLLERR?
    return events;
}



int TcpClient::connect(int port, std::string address) {
    if(connected) return -2;
    try {
        socket = std::make_shared<TCPSocket>(address, port);
        socket->setEvents(POLLIN);
        connected = true;
    }
    catch (SocketException &e) {
        std::cerr << "TcpClient: " << e.what() << std::endl;
        connected = false;
        return -1;
    }
    return 0;
}



void TcpClient::disconnect() {
    if (socket != nullptr && connected) {
        try {
            socket->disconnect();
        } catch(SocketException) {
            // recipient disconnected first, ignore for now
            std::cerr << "Peer disconnected first" << std::endl;
        }
        connected = false;
    }
}



bool TcpClient::isConnected() {
    if(connected) {
        short isr;
        socket->poll(isr, -1);
        checkFlags(isr);
        return connected;
    }
    return false;
}



void TcpClient::setTimeout(int time_ms) {
	// -1 means waiting indefinitely ie no timeout
	if (time_ms < -1) timeout_ = -1; // normalize
	else              timeout_ = time_ms;
}



int TcpClient::poll(int& isr, int timeout) {
    short rsi;
    int result = socket->poll(rsi, timeout);
    isr = rsi;
    return result;
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
        if(socket->poll(isr, timeout_) == 0) {
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