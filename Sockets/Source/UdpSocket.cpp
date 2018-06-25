/**
 * Created by TekuConcept on June 21, 2018
 */

#include "UdpSocket.h"
#include "SocketInterface.h"
#include <stdexcept>

using namespace Impact;


UdpSocket::UdpSocket() {
	initialize();
}


UdpSocket::UdpSocket(unsigned short port) {
	initialize();
	try { open(port); }
	catch (std::runtime_error e) {
		std::string message("UdpSocket::UdpSocket(2)\n");
		message.append(e.what());
		throw std::runtime_error(message);
	}
}


UdpSocket::UdpSocket(unsigned short port, const std::string& address) {
	initialize();
	try { open(port, address); }
	catch (std::runtime_error e) {
		std::string message("UdpSocket::UdpSocket(3)\n");
		message.append(e.what());
		throw std::runtime_error(message);
	}
}


UdpSocket::~UdpSocket() {
	if (_isOpen_) {
		try { close(); }
		catch (...) { /* do nothing - swallow error */ }
	}
}


void UdpSocket::initialize() {
	_isOpen_ = false;
}


void UdpSocket::open(std::function<void ()> configure) {
	if(_isOpen_) {
		throw std::runtime_error(
			"UdpSocket::open({})\nUDP socket already open.");
	}
	else {
		try {
			_handle_ = SocketInterface::create(SocketDomain::INET,
				SocketType::DATAGRAM, SocketProtocol::UDP);
			if(configure) configure();
			SocketInterface::setBroadcast(_handle_, true);
		}
		catch (std::runtime_error e) {
			std::string message("UdpSocket::open({})\n");
			message.append(e.what());
			throw std::runtime_error(message);
		}

		_isOpen_ = true;
	}
}


void UdpSocket::open() {
	try { open(nullptr); }
	catch (...) { throw; }
}


void UdpSocket::open(unsigned short port) {
	try { open([&](){ SocketInterface::setLocalPort(_handle_, port); }); }
	catch (...) { throw; }
}


void UdpSocket::open(unsigned short port, const std::string& address) {
	try {
		open([&](){
			SocketInterface::setLocalAddressAndPort(
				_handle_, address, port);
		});
	}
	catch (...) { throw; }
}


bool UdpSocket::is_open() const {
	return _isOpen_;
}


void UdpSocket::close() {
	if(!_isOpen_) {
		throw std::runtime_error(
			"UdpSocket::close()\nUDP socket already closed.");
	}
	else {
		try {
			SocketInterface::close(_handle_);
			_isOpen_ = false;
		}
		catch (std::runtime_error e) {
			std::string message("UdpServer::close()\n");
			message.append(e.what());
			throw std::runtime_error(message);
		}
	}
}


int UdpSocket::sendTo(const void* buffer, int length,
	unsigned short targetPort, const std::string& targetAddress) {
	try {
		return SocketInterface::sendto(
			_handle_, buffer, length,
			targetPort, targetAddress
		);
	}
	catch (std::runtime_error e) {
		std::string message("UdpSocket::sendTo()\n");
		message.append(e.what());
		throw std::runtime_error(message);
	}
}


int UdpSocket::recvFrom(void* buffer, int length,
	unsigned short& sourcePort, std::string& sourceAddress) {
	try {
		return SocketInterface::sendto(
			_handle_, buffer, length,
			sourcePort, sourceAddress
		);
	}
	catch (std::runtime_error e) {
		std::string message("UdpSocket::recvFrom()\n");
		message.append(e.what());
		throw std::runtime_error(message);
	}
}


void UdpSocket::setMulticastTTL(unsigned char ttl) {
	try { SocketInterface::setMulticastTTL(_handle_, ttl); }
	catch (std::runtime_error e) {
		std::string message("UdpSocket::setMulticastTTL()\n");
		message.append(e.what());
		throw std::runtime_error(message);
	}
}


void UdpSocket::group(const std::string& group, GroupApplication method) {
	try { SocketInterface::group(_handle_, group, method); }
	catch (std::runtime_error e) {
		std::string message("UdpSocket::group()\n");
		message.append(e.what());
		throw std::runtime_error(message);
	}
}