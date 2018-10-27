/**
 * Created by TekuConcept on July 8, 2018
 */

#include "sockets/socketstream.h"

#include <stdexcept>

#include "utils/impact_error.h"
#include "sockets/generic.h"

using namespace impact;

#define SUCCESS 0
#define FAIL   -1

#define VERBOSE(x) std::cout << x << std::endl

socketstream::socketstream(
    basic_socket& __socket,
    unsigned int  __stream_buffer_size)
: std::iostream(this), m_handle_(__socket)
{
    if (__socket.type() != socket_type::STREAM)
        throw impact_error("Socket is not streamable");

    _M_initialize(__stream_buffer_size);
}


socketstream::~socketstream()
{
    if (m_output_buffer_ != NULL) {
        delete[] m_output_buffer_;
        m_output_buffer_ = NULL;
    }

    if(m_input_buffer_ != NULL) {
        delete[] m_input_buffer_;
        m_input_buffer_ = NULL;
    }
}


void
socketstream::_M_initialize(unsigned int __buffer_size)
{
    m_stream_buffer_size_ = __buffer_size < 256 ? 256 : __buffer_size;
    m_output_buffer_      = new char[m_stream_buffer_size_ + 1];
    m_input_buffer_       = new char[m_stream_buffer_size_ + 1];

    m_hangup_  = false;
    m_timeout_ = -1;

    setp(m_output_buffer_, m_output_buffer_ + m_stream_buffer_size_ - 1);
    setg(m_input_buffer_, m_input_buffer_ + m_stream_buffer_size_ - 1,
        m_input_buffer_ + m_stream_buffer_size_ - 1);

    struct poll_handle handle;
    handle.socket = m_handle_.get();
    handle.events = (int)poll_flags::IN;
    m_poll_handle_.push_back(handle);
}


int
socketstream::sync()
{
    return _M_write_base(SUCCESS);
}


int
socketstream::underflow()
{
    if (!m_handle_)
        return EOF;

    if (m_hangup_) {
        setstate(std::ios_base::badbit);
        return EOF;
    }

    try {
        auto status = impact::poll(&m_poll_handle_, m_timeout_);

        if (status == 0)
            return EOF; // timeout

        short flags = m_poll_handle_[0].return_events;
        m_poll_handle_[0].return_events = 0;

        if ((int)(flags & (int)poll_flags::HANGUP)) {
            m_hangup_ = true;
            setstate(std::ios_base::badbit);
            return EOF;
        }

        if ((int)(flags & (int)poll_flags::IN)) {
            int bytes_received = m_handle_.recv(eback(), m_stream_buffer_size_);

            if (bytes_received == 0)
                return EOF;

            setg(eback(), eback(), eback() + bytes_received);
            return *eback();
        }
    }
    catch (...) { return EOF; }

    return EOF;
}


int
socketstream::overflow(int __c)
{
    return _M_write_base(__c);
}


int
socketstream::_M_write_base(int __c)
{
    if (!m_handle_)
        return EOF;

    if (m_hangup_) {
        setstate(std::ios_base::badbit);
        return EOF;
    }

    try {
        auto length = int(pptr() - pbase());
        m_handle_.send(pbase(), length);
        setp(pbase(), epptr());
    }
    catch (...) { return EOF; }

    return __c;
}


bool
socketstream::hup() const noexcept
{
    return m_hangup_;
}


void
socketstream::set_timeout(int __milliseconds) noexcept
{
    m_timeout_ = __milliseconds<-1?-1:__milliseconds;
}
