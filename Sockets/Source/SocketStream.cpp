/**
 * Created by TekuConcept on June 19, 2018
 */

#include "SocketStream.h"

using namespace Impact;

std::shared_ptr<SocketStream> SocketStream::createTcpSocket(
			unsigned int streamBufferSize) {
	return std::make_shared<SocketStream>(SocketProtocol::TCP,
		streamBufferSize);
}


SocketStream::SocketStream(SocketProtocol protocol,
	unsigned int streamBufferSize) :
    std::iostream(this),
    _handle_(SocketType::STREAM, protocol),
    _streamBufferSize_(streamBufferSize<256?256:streamBufferSize),
    _outputBuffer_(new char[_streamBufferSize_ + 1]),
    _inputBuffer_(new char[_streamBufferSize_ + 1]) {
	initialize();
}


SocketStream::SocketStream(SocketHandle&& handle,
	unsigned int streamBufferSize) :
	_handle_(std::move(handle)),
    _streamBufferSize_(streamBufferSize<256?256:streamBufferSize),
    _outputBuffer_(new char[_streamBufferSize_ + 1]),
    _inputBuffer_(new char[_streamBufferSize_ + 1]) {
	initialize();
}


SocketStream::~SocketStream() {
	if(_outputBuffer_ != NULL) {
		delete[] _outputBuffer_;
		_outputBuffer_ = NULL;
	}
	if(_inputBuffer_ != NULL) {
		delete[] _inputBuffer_;
		_inputBuffer_ = NULL;
	}
}


void SocketStream::initialize() {
	setp(_outputBuffer_, _outputBuffer_ + _streamBufferSize_ - 1);
    setg(_inputBuffer_, _inputBuffer_ + _streamBufferSize_ - 1,
    	_inputBuffer_ + _streamBufferSize_ - 1);
}