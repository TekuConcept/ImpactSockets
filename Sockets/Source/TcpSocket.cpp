/**
 * Created by TekuConcept on June 19, 2018
 */

#include "TcpSocket.h"

using namespace Impact;

TcpSocket::TcpSocket(unsigned int streamBufferSize) :
    std::iostream(this),
    _handle_(SocketType::STREAM, SocketProtocol::TCP),
    _streamBufferSize_(streamBufferSize<256?256:streamBufferSize),
    _outputBuffer_(new char[_streamBufferSize_ + 1]),
    _inputBuffer_(new char[_streamBufferSize_ + 1]) {
	initialize();
}


TcpSocket::TcpSocket(SocketHandle&& handle, unsigned int streamBufferSize) :
	_handle_(std::move(handle)),
    _streamBufferSize_(streamBufferSize<256?256:streamBufferSize),
    _outputBuffer_(new char[_streamBufferSize_ + 1]),
    _inputBuffer_(new char[_streamBufferSize_ + 1]) {
	initialize();
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


void TcpSocket::initialize() {
	setp(_outputBuffer_, _outputBuffer_ + _streamBufferSize_ - 1);
    setg(_inputBuffer_, _inputBuffer_ + _streamBufferSize_ - 1,
    	_inputBuffer_ + _streamBufferSize_ - 1);
}