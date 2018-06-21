/**
 * Created by TekuConcept on June 19, 2018
 */

#include "TcpSocket.h"
#include "SocketInterface.h"

using namespace Impact;
#define SUCCESS  0
#define FAIL    -1


TcpSocket::TcpSocket(unsigned int streamBufferSize) :
    std::iostream(this) {
	initialize(streamBufferSize);
}


TcpSocket::TcpSocket(int port, std::string address,
	unsigned int streamBufferSize) :
    std::iostream(this) {
	initialize(streamBufferSize);
	open(port, address);
}


TcpSocket::~TcpSocket() {
	if(_outputBuffer_ != NULL) {
		delete[] _outputBuffer_;
		_outputBuffer_ = NULL;
	}
	if(_inputBuffer_ != NULL) {
		delete[] _inputBuffer_;
		_inputBuffer_ = NULL;
	}
}


void TcpSocket::initialize(unsigned int bufferSize) {
	_streamBufferSize_ = bufferSize<256?256:bufferSize;
	_outputBuffer_     = new char[_streamBufferSize_ + 1];
    _inputBuffer_      = new char[_streamBufferSize_ + 1];

    _isOpen_  = false;
    _hangup_  = false;
    _timeout_ = -1;

	setp(_outputBuffer_, _outputBuffer_ + _streamBufferSize_ - 1);
    setg(_inputBuffer_, _inputBuffer_ + _streamBufferSize_ - 1,
    	_inputBuffer_ + _streamBufferSize_ - 1);
}


void TcpSocket::open(int port, std::string address) {
	if(_isOpen_) { setstate(std::ios_base::failbit); }
	else {
		try {
			_handle_ = SocketInterface::create(SocketDomain::INET,
				SocketType::STREAM, SocketProtocol::TCP);
			SocketInterface::connect(_handle_, address, port);
			_pollTable_.push_back({_handle_,PollFlags::IN});
			clear();
		}
	    catch (...) { setstate(std::ios_base::failbit); }

		_isOpen_ = true;
		_hangup_ = false;
	}
}


bool TcpSocket::is_open() const { return _isOpen_; }


void TcpSocket::close() {
	if(!_isOpen_) { setstate(std::ios_base::failbit); }
	else {
		try {
			SocketInterface::close(_handle_);
			_pollTable_.pop_back();
			_isOpen_ = false;
		}
		catch (...) { setstate(std::ios_base::failbit); }
	}
}


int TcpSocket::sync() {
	return writeBase(SUCCESS);
}


int TcpSocket::underflow() {
	if(!_isOpen_) { return EOF; }
	if(_hangup_) {
		setstate(std::ios_base::badbit);
		return EOF;
	}

	try {
		auto status = SocketInterface::poll(_pollTable_, _timeout_);
		if(status <= 0) return EOF; // timeout or error
		auto flags = _pollTable_[0];
		_pollTable_.resetEvents();

		if((int)(flags & PollFlags::HANGUP)) {
			_hangup_ = true;
			setstate(std::ios_base::badbit);
			return EOF;
		}

		if((int)(flags & PollFlags::IN)) {
			int bytesReceived = SocketInterface::recv(
				_handle_, eback(), _streamBufferSize_);
			if(bytesReceived == 0) return EOF;

			setg(eback(), eback(), eback() + bytesReceived);
			return *eback();
		}
	}
	catch (...) { return EOF; }

	return EOF;
}


int TcpSocket::overflow(int c) {
	return writeBase(c);
}


int TcpSocket::writeBase(int c) {
	if(!_isOpen_) { return EOF; }
	if(_hangup_) {
		setstate(std::ios_base::badbit);
		return EOF;
	}

	try {
		auto length = int(pptr() - pbase());
		SocketInterface::send(_handle_, pbase(), length);
		setp(pbase(), epptr());
	}
	catch (...) { return EOF; }

	return c;
}


bool TcpSocket::hup() const {
	return _hangup_;
}


void TcpSocket::setTimeout(int milliseconds) {
	_timeout_ = milliseconds<-1?-1:milliseconds;
}