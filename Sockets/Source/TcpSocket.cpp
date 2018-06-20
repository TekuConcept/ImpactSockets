/**
 * Created by TekuConcept on June 19, 2018
 */

#include "TcpSocket.h"
#include "SocketInterface.h"

using namespace Impact;


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

    _isOpen_ = false;

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
			clear();
		}
	    catch (...) { setstate(std::ios_base::failbit); }

		_isOpen_ = true;
	}
}


bool TcpSocket::is_open() const { return _isOpen_; }


void TcpSocket::close() {
	if(!_isOpen_) { setstate(std::ios_base::failbit); }
	else {
		try {
			SocketInterface::close(_handle_);
			_isOpen_ = false;
		}
		catch (...) { setstate(std::ios_base::failbit); }
	}
}