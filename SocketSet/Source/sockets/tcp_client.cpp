/**
 * Created by TekuConcept on February 11, 2021
 */

#include "sockets/tcp_client.h"

using namespace impact;


tcp_client::tcp_client(std::shared_ptr<event_loop_interface> __event_loop)
{ m_base = __event_loop->create_tcp_client(); }


tcp_address_t
tcp_client::address() const
{ return m_base->address(); }


size_t
tcp_client::bytes_read() const
{ return m_base->bytes_read(); }


size_t
tcp_client::bytes_written() const
{ return m_base->bytes_written(); }


bool
tcp_client::connecting() const
{ return m_base->connecting(); }


bool
tcp_client::destroyed() const
{ return m_base->destroyed(); }


std::string
tcp_client::local_address() const
{ return m_base->local_address(); }


unsigned short
tcp_client::local_port() const
{ return m_base->local_port(); }


bool
tcp_client::pending() const
{ return m_base->pending(); }


std::string
tcp_client::remote_address() const
{ return m_base->remote_address(); }


address_family
tcp_client::remote_family() const
{ return m_base->remote_family(); }


unsigned short
tcp_client::remote_port() const
{ return m_base->remote_port(); }


size_t
tcp_client::timeout() const
{ return m_base->timeout(); }


std::string
tcp_client::ready_state() const
{ return m_base->ready_state(); }


tcp_client_interface*
tcp_client::connect(
    std::string               __path,
    event_emitter::callback_t __cb)
{ return m_base->connect(__path, __cb); }


tcp_client_interface*
tcp_client::connect(
    unsigned short            __port,
    event_emitter::callback_t __cb)
{ return m_base->connect(__port, __cb); }


tcp_client_interface*
tcp_client::connect(
    unsigned short            __port,
    std::string               __host,
    event_emitter::callback_t __cb)
{ return m_base->connect(__port, __host, __cb); }


tcp_client_interface*
tcp_client::destroy(std::string __error)
{ return m_base->destroy(__error); }


tcp_client_interface*
tcp_client::end(event_emitter::callback_t __cb)
{ return m_base->end(__cb); }


tcp_client_interface*
tcp_client::end(
    std::string               __data,
    event_emitter::callback_t __cb)
{ return m_base->end(__data, __cb); }


tcp_client_interface*
tcp_client::end(
    std::string               __data,
    std::string               __encoding,
    event_emitter::callback_t __cb)
{ return m_base->end(__data, __encoding, __cb); }


tcp_client_interface*
tcp_client::pause()
{ return m_base->pause(); }


tcp_client_interface*
tcp_client::resume()
{ return m_base->resume(); }


tcp_client_interface*
tcp_client::set_encoding(std::string __encoding)
{ return m_base->set_encoding(__encoding); }


tcp_client_interface*
tcp_client::set_keep_alive(unsigned int __initial_delay)
{ return m_base->set_keep_alive(__initial_delay); }


tcp_client_interface*
tcp_client::set_keep_alive(
    bool         __enable,
    unsigned int __initial_delay)
{ return m_base->set_keep_alive(__enable, __initial_delay); }


tcp_client_interface*
tcp_client::set_no_delay(bool __no_delay)
{ return m_base->set_no_delay(__no_delay); }


tcp_client_interface*
tcp_client::set_timeout(
    unsigned int              __timeout,
    event_emitter::callback_t __cb)
{ return m_base->set_timeout(__timeout, __cb); }


bool
tcp_client::write(
    std::string               __data,
    event_emitter::callback_t __cb)
{ return m_base->write(__data, __cb); }


bool
tcp_client::write(
    std::string               __data,
    std::string               __encoding,
    event_emitter::callback_t __cb)
{ return m_base->write(__data, __encoding, __cb); }
