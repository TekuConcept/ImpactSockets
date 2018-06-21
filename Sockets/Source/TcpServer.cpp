/**
 *	Created by TekuConcept on May 12, 2017
 */

#include "TcpServer.h"
#include "SocketInterface.h"
#include <stdexcept>

using namespace Impact;


TcpServer::TcpServer() {
	initialize();
}


TcpServer::TcpServer(unsigned short port, int backlog) {
	initialize();
	try { open(port, backlog); }
	catch (std::runtime_error e) {
		std::string message("TcpServer::TcpServer(2)\n");
		message.append(e.what());
		throw std::runtime_error(message);
	}
}


TcpServer::TcpServer(unsigned short port, const std::string& address,
	int backlog) {
	initialize();
	try { open(port, address, backlog); }
	catch (std::runtime_error e) {
		std::string message("TcpServer::TcpServer(3)\n");
		message.append(e.what());
		throw std::runtime_error(message);
	}
}


TcpServer::~TcpServer() {
	if(_isOpen_) {
		try { close(); } catch (...) {}
		_isOpen_ = false;
	}
}


void TcpServer::initialize() {
	_isOpen_ = false;
}


void TcpServer::open(std::function<void()> configure, int backlog) {
	if(_isOpen_) {
		throw std::runtime_error("TcpServer::open(1)\nServer already open.");
	}
	else {
		try {
			_handle_ = SocketInterface::create(SocketDomain::INET,
				SocketType::STREAM, SocketProtocol::TCP);
			configure();
			SocketInterface::listen(_handle_, backlog);
		}
		catch (std::runtime_error e) {
			std::string message("TcpServer::open(1)\n");
			message.append(e.what());
			throw std::runtime_error(message);
		}

		_isOpen_ = true;
	}
}


void TcpServer::open(unsigned short port, int backlog) {
	try {
		open([&](){
			SocketInterface::setLocalPort(_handle_, port);
		}, backlog);
	}
	catch (...) { throw; }
}


void TcpServer::open(unsigned short port, const std::string& address,
	int backlog) {
	try {
		open([&](){
			SocketInterface::setLocalAddressAndPort(
				_handle_, address, port);
		}, backlog);
	}
	catch (...) { throw; }
}


bool TcpServer::is_open() const {
	return _isOpen_;
}


void TcpServer::close() {
	if(!_isOpen_) {
		throw std::runtime_error("TcpServer::close()\nServer not open.");
	}
	else {
		try {
			SocketInterface::close(_handle_);
			_isOpen_ = false;
		}
		catch (std::runtime_error e) {
			std::string message("TcpServer::close()\n");
			message.append(e.what());
			throw std::runtime_error(message);
		}
	}
}


void TcpServer::accept(TcpSocket& socket) {
	if(!_isOpen_) {
		throw std::runtime_error("TcpServer::accept()\nServer not open.");
	}
	else {
		try {
			SocketInterface::accept(_handle_, socket._handle_);
			socket._pollTable_.push_back({socket._handle_,PollFlags::IN});
			socket._isOpen_ = true;
		}
		catch (std::runtime_error e) {
			std::string message("TcpServer::accept()\n");
			message.append(e.what());
			throw std::runtime_error(message);
		}
	}
}


int TcpServer::port() {
	if(!_isOpen_) {
		throw std::runtime_error("TcpServer::port()\nServer not open.");
	}
	else {
		try {
			return SocketInterface::getLocalPort(_handle_);
		}
		catch (std::runtime_error e) {
			std::string message("TcpServer::port()\n");
			message.append(e.what());
			throw std::runtime_error(message);
		}
	}

	return -1;
}
