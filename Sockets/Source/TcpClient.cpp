/**
*  Created by TekuConcept on Mar 29, 2016
*/

#include "TcpClient.h"
#include <iostream>
#include <stdexcept>

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



TcpClient::~TcpClient() {
	delete[] outputBuffer_;
	delete[] inputBuffer_;
}



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
	if (connected) { // test for POLLHUP
		pollToken.reset();
		if(Socket::poll(pollToken, 0) < 0) {
			// Todo: poll error handling?
			std::cout << "TCP Client: Connection poll failed with error code ";
			std::cout << errno << " (";
			std::cout << std::strerror(errno) << ")" << std::endl;
		}
		checkFlags(pollToken[0]);
	}
	// TODO if possible: PSH null tcp frame in an attempt
	// to exploit broken connections
	return connected;
}



int TcpClient::sync() {
	try {
		int len = int(pptr() - pbase());
		if (socket != nullptr && connected)
			socket->send(pbase(), len);
		setp(pbase(), epptr());
		return 0;
	} catch(SocketException) {
		return -1;
	}
}



int TcpClient::underflow() {
	if (socket != nullptr && connected) {
		pollToken.reset();
		auto status = Socket::poll(pollToken, timeout);
		if (status < 0) {
			std::cout << "TCP Client: Poll failed with error code ";
			std::cout << errno << " (";
			std::cout << std::strerror(errno) << ")" << std::endl;
			connected = false;
		}
		else if (status == 0) {
			EventArgs e;
			onTimeout.invoke(self, e);
		}
		else if ((checkFlags(pollToken[0]) & POLLIN) > 0) {
			int bytesReceived = socket->recv(eback(), BUF_SIZE);
			if(bytesReceived == 0) {
				connected = false;
				return EOF;
			}
			else {
				setg(eback(), eback(), eback() + bytesReceived);
				return *eback();
			}
		}
	}
	return EOF;
}



void TcpClient::setTimeout(int time_ms) {
	// -1 means waiting indefinitely ie no timeout
	if (time_ms < -1) timeout = -1; // normalize
	else              timeout = time_ms;
}



void TcpClient::send(const void *buffer, int bufferLen, int flags)
throw(SOC_EXCEPTION) {
	socket->send(buffer,bufferLen,flags);
}



int TcpClient::recv(void *buffer, int bufferLen, int flags)
throw(SOC_EXCEPTION) {
	return socket->recv(buffer,bufferLen,flags);
}



SocketHandle& TcpClient::getHandle() {
	return socket->getHandle();
}