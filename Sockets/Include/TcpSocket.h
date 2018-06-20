/**
 * Created by TekuConcept on June 19, 2018
 */

#ifndef _TCP_SOCKET_H_
#define _TCP_SOCKET_H_

#include "SocketHandle.h"
#include <string>
#include <streambuf>
#include <iostream>
#include <memory>

namespace Impact {
	class TcpSocket :
	private std::streambuf, public std::iostream {
		SocketHandle _handle_;

		unsigned int _streamBufferSize_;
		char* _outputBuffer_;
		char* _inputBuffer_;

		bool _isOpen_;

		void initialize(unsigned int);

	public:
		TcpSocket(unsigned int streamBufferSize=256);
		TcpSocket(int port, std::string address="12.0.0.1",
			unsigned int streamBufferSize=256);
		virtual ~TcpSocket();

		void open(int port, std::string address="12.0.0.1");
		bool is_open() const;
		void close();

		// int sync();
		// int underflow();
		// int overflow(int c = EOF);
	};
}

#endif