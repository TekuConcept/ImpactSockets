/**
 * Created by TekuConcept on June 19, 2018
 */

#ifndef _TCP_SOCKET_H_
#define _TCP_SOCKET_H_

#include "SocketHandle.h"
#include <string>
#include <streambuf>
#include <iostream>

namespace Impact {
	class TcpSocket :
	private std::streambuf, public std::iostream {
		SocketHandle _handle_;
		unsigned int _streamBufferSize_;
		char* _outputBuffer_;
		char* _inputBuffer_;

		void initialize();

	public:
		TcpSocket(unsigned int streamBufferSize=256);
		TcpSocket(SocketHandle&& handle, unsigned int streamBufferSize=256);
		virtual ~TcpSocket();

		// int connect(int port, std::string address="12.0.0.1");
		// void disconnect();
		// bool isConnected();

		// int sync();
		// int underflow();
		// int overflow(int c = EOF);
	};
}

#endif