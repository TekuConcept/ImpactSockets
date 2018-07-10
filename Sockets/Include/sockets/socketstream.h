/**
 * Created by TekuConcept on July 8, 2018
 */

#ifndef _SOCKETSTREAM_H_
#define _SOCKETSTREAM_H_

#include <string>
#include <streambuf>
#include <iostream>

#include "sockets/basic_socket.h"
#include "sockets/poll_vector.h"

namespace impact {
	class socketstream : private std::streambuf, public std::iostream {
	public:
		socketstream(basic_socket& socket, unsigned int stream_buffer_size = 256)
			/* throw(std::runtime_error) */;
		socketstream(const socketstream&) = delete;
		socketstream& operator=(const socketstream&) = delete;
		virtual ~socketstream();

		virtual int sync();
		virtual int underflow();
		virtual int overflow(int c = EOF);

		bool hup() const noexcept;
		void set_timeout(int milliseconds) noexcept;

	private:
		basic_socket    m_handle_;
		poll_vector     m_poll_handle_;

		unsigned int    m_stream_buffer_size_;
		char*           m_output_buffer_;
		char*           m_input_buffer_;

		bool            m_hangup_;
		bool            m_again_;
		int             m_timeout_;

		void _M_initialize(unsigned int);
		void _M_check_hangup();
		int  _M_write_base(int c);
	};
}

#undef LOCAL
#endif
