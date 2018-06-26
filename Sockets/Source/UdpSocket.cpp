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
			_pollTable_.push_back({_handle_,PollFlags::IN});
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
			_pollTable_.pop_back();
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


int UdpSocket::recvFrom(void* buffer, int length, unsigned short& sourcePort,
	std::string& sourceAddress, int timeout) {
	try {
		auto status = SocketInterface::poll(_pollTable_, timeout);
		if(status == 0) return 0;
		auto flags = _pollTable_[0];
		_pollTable_.resetEvents();

		if((int)(flags & PollFlags::IN)) {
			return SocketInterface::recvfrom(
				_handle_, buffer, length,
				sourcePort, sourceAddress
			);
		} else return 0;
	}
	catch (std::runtime_error e) {
		std::string message("UdpSocket::recvFrom()\n");
		message.append(e.what());
		throw std::runtime_error(message);
	}
}


void UdpSocket::setBroadcast(bool enabled) {
	try { SocketInterface::setBroadcast(_handle_, enabled); }
	catch (std::runtime_error e) {
		std::string message("UdpSocket::setBroadcast()\n");
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