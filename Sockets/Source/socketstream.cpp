/**
 * Created by TekuConcept on July 8, 2018
 */

#include "socketstream.h"
#include "SocketProbe.h"
#include <stdexcept>

using namespace Impact;

#define SUCCESS	0
#define FAIL	-1
#define VERBOSE(x) std::cout << x << std::endl

socketstream::socketstream(basic_socket socket, unsigned int streamBufferSize) :
  std::iostream(this), _handle_(socket) {
  if(socket.type() != SocketType::STREAM) {
    throw std::runtime_error(
      "socketstream::socketstream()\n"
      "Socket is not a streamable"
    );
  }
	initialize(streamBufferSize);
}


socketstream::~socketstream() {
	if(_outputBuffer_ != NULL) {
		delete[] _outputBuffer_;
		_outputBuffer_ = NULL;
	}
	if(_inputBuffer_ != NULL) {
		delete[] _inputBuffer_;
		_inputBuffer_ = NULL;
	}
}


void socketstream::initialize(unsigned int bufferSize) {
	_streamBufferSize_ = bufferSize<256?256:bufferSize;
	_outputBuffer_     = new char[_streamBufferSize_ + 1];
  _inputBuffer_      = new char[_streamBufferSize_ + 1];

  _hangup_  = false;
  _timeout_ = -1;

	setp(_outputBuffer_, _outputBuffer_ + _streamBufferSize_ - 1);
  setg(_inputBuffer_, _inputBuffer_ + _streamBufferSize_ - 1,
  	_inputBuffer_ + _streamBufferSize_ - 1);

  _pollTable_.push_back({_handle_,PollFlags::IN});
}


int socketstream::sync() {
	return write_base(SUCCESS);
}


int socketstream::underflow() {
	if(!_handle_) return EOF;

	if(_hangup_) {
		setstate(std::ios_base::badbit);
		return EOF;
	}

	try {
		auto status = SocketProbe::poll(_pollTable_, _timeout_);
		if(status == 0) return EOF; // timeout
		auto flags = _pollTable_[0];
		_pollTable_.resetEvents();

		if((int)(flags & PollFlags::HANGUP)) {
			_hangup_ = true;
			setstate(std::ios_base::badbit);
			return EOF;
		}

		if((int)(flags & PollFlags::IN)) {
			int bytesReceived = _handle_.recv(eback(), _streamBufferSize_);
			if(bytesReceived == 0) return EOF;

			setg(eback(), eback(), eback() + bytesReceived);
			return *eback();
		}
	}
	catch (std::exception e) { return EOF; }

	return EOF;
}


int socketstream::overflow(int c) {
	return write_base(c);
}


int socketstream::write_base(int c) {
	if(!_handle_) return EOF;

  if(_hangup_) {
		setstate(std::ios_base::badbit);
		return EOF;
	}

	try {
		auto length = int(pptr() - pbase());
		_handle_.send(pbase(), length);
		setp(pbase(), epptr());
	}
	catch (std::exception e) {
    return EOF;
  }

	return c;
}


bool socketstream::hup() const noexcept {
	return _hangup_;
}


void socketstream::set_timeout(int milliseconds) noexcept {
	_timeout_ = milliseconds<-1?-1:milliseconds;
}
