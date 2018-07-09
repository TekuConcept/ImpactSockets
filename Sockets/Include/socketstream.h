/**
 * Created by TekuConcept on July 8, 2018
 */

#ifndef _SOCKETSTREAM_H_
#define _SOCKETSTREAM_H_

#include "basic_socket.h"
#include "SocketPollTable.h"
#include <string>
#include <streambuf>
#include <iostream>

namespace Impact {
  class socketstream : private std::streambuf, public std::iostream {
    basic_socket _handle_;
    SocketPollTable _pollTable_;

    unsigned int _streamBufferSize_;
		char* _outputBuffer_;
		char* _inputBuffer_;

		bool _hangup_;
    bool _again_;
		int  _timeout_;

    void initialize(unsigned int);
		void check_hangup();
		int write_base(int c);

  public:
    socketstream(basic_socket socket, unsigned int streamBufferSize=256)
      /* throw(std::runtime_error) */;
    socketstream(const socketstream&) = delete;
    socketstream& operator=(const socketstream&) = delete;
		virtual ~socketstream();

		virtual int sync();
		virtual int underflow();
		virtual int overflow(int c = EOF);

		bool hup() const noexcept;
		void set_timeout(int milliseconds) noexcept;
  };
}

#undef LOCAL
#endif
