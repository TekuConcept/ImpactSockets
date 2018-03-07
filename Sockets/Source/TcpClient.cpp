/**
*  Created by TekuConcept on Mar 29, 2016
*/

#include "TcpClient.h"
#include <iostream>

#if defined(_MSC_VER)
    #include <windows.h>
    #if defined(errno)
        #undef errno
    #endif
    #define errno WSAGetLastError()
#endif

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
	timeout = -1;

	setp(outputBuffer_, outputBuffer_ + BUF_SIZE - 1);
	setg(inputBuffer_, inputBuffer_ + BUF_SIZE - 1, inputBuffer_ + BUF_SIZE - 1);
}



short TcpClient::checkFlags(short events) {
	// set internal flags to handle user actions in advance
	if ((events & POLLHUP) != 0)
		connected = false;
	// handle POLLERR?
	return events;
}



int TcpClient::connect(int port, std::string address) {
	if (connected) return -2;
	try {
		socket = std::make_shared<TCPSocket>(address, port);
		connected = true;
		pollToken.add(socket->getHandle(), POLLIN);
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
		}
		catch (SocketException) {
			// recipient disconnected first, ignore for now
			std::cerr << "Peer disconnected first" << std::endl;
		}
		connected = false;
	}
}



bool TcpClient::isConnected() {
	if (connected) {
		pollToken.reset();
		Socket::poll(pollToken, 0); // Todo: poll error handling?
		checkFlags(pollToken[0]);
		return connected;
	}
	return false;
}



int TcpClient::sync() {
	int len = int(pptr() - pbase());
	if (socket != nullptr && connected)
		socket->send(pbase(), len);
	setp(pbase(), epptr());
	return 0;
}



int TcpClient::underflow() {
	if (socket != nullptr && connected) {
		pollToken.reset();
		auto status = Socket::poll(pollToken, timeout);
		if (status < 0) {
			std::cout << "TCP Client: Poll failed with error code ";
			std::cout << errno << " (";
			std::cout << std::strerror(errno) << ")" << std::endl;
		}
		else if (status == 0) {
			EventArgs e;
			onTimeout.invoke(self, e);
		}
		else if ((checkFlags(pollToken[0]) & POLLIN) > 0) {
			int bytesReceived = socket->recv(eback(), BUF_SIZE);
			setg(eback(), eback(), eback() + bytesReceived);
			return *eback();
		}
	}
	return EOF;
}



void TcpClient::setTimeout(int time_ms) {
	// -1 means waiting indefinitely ie no timeout
	if (time_ms < -1) timeout = -1; // normalize
	else              timeout = time_ms;
}



SocketHandle& TcpClient::getHandle() {
	return socket->getHandle();
}