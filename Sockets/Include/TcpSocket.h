/**
 * Created by TekuConcept on June 19, 2018
 */

#ifndef _TCP_SOCKET_H_
#define _TCP_SOCKET_H_

#include "basic_socket.h"
#include "SocketPollTable.h"
#include <string>
#include <streambuf>
#include <iostream>

#define LOCAL "localhost"

namespace Impact {
	class TcpSocket : private std::streambuf, public std::iostream {
		basic_socket _handle_;
		SocketPollTable _pollTable_;

		unsigned int _streamBufferSize_;
		char* _outputBuffer_;
		char* _inputBuffer_;

		bool _isOpen_;
		bool _hangup_;
		int  _timeout_;

		void initialize(unsigned int);
		void checkHangup();
		int writeBase(int c);

	public:
		TcpSocket(unsigned int streamBufferSize=256);
		TcpSocket(int port, std::string address=LOCAL,
			unsigned int streamBufferSize=256);
		virtual ~TcpSocket();

		virtual void open(int port, std::string address=LOCAL);
		bool is_open() const;
		virtual void close();

		virtual int sync();
		virtual int underflow();
		virtual int overflow(int c = EOF);
		bool hup() const;

		void setTimeout(int milliseconds);

		friend class TcpServer;
	};
}

#undef LOCAL
#endif
