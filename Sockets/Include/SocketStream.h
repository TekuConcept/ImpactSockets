/**
 * Created by TekuConcept on June 19, 2018
 */

#ifndef _SOCKET_STREAM_H_
#define _SOCKET_STREAM_H_

#include "SocketHandle.h"
#include <string>
#include <streambuf>
#include <iostream>
#include <memory>

namespace Impact {
	class SocketStream :
	private std::streambuf, public std::iostream {
		SocketHandle _handle_;
		unsigned int _streamBufferSize_;
		char* _outputBuffer_;
		char* _inputBuffer_;

		void initialize();

	public:
		SocketStream(SocketProtocol protocol,
			unsigned int streamBufferSize=256);
		SocketStream(SocketHandle&& handle,
			unsigned int streamBufferSize=256);
		virtual ~SocketStream();

		// int connect(int port, std::string address="12.0.0.1");
		// void disconnect();
		// bool isConnected();

		// int sync();
		// int underflow();
		// int overflow(int c = EOF);

		static std::shared_ptr<SocketStream> createTcpSocket(
			unsigned int streamBufferSize);
	};
}

#endif