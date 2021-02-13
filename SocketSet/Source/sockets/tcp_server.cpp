/**
 * Created by TekuConcept on January 19, 2021
 */

#include "sockets/tcp_server.h"

using namespace impact;


tcp_server::tcp_server(
    std::shared_ptr<event_loop_interface> __event_loop)
{ m_base = __event_loop->create_tcp_server(); }


tcp_address_t
tcp_server::address() const
{ return m_base->address(); }


bool
tcp_server::listening() const
{ return m_base->listening(); }


size_t
tcp_server::max_connections() const
{ return m_base->max_connections(); }


void
tcp_server::max_connections(size_t __value)
{ m_base->max_connections(__value); }


void
tcp_server::close(event_emitter::callback_t __cb)
{ m_base->close(__cb); }


void
tcp_server::listen(
    std::string               __path,
    event_emitter::callback_t __cb)
{ m_base->listen(__path, __cb); }


void
tcp_server::listen(
    unsigned short            __port,
    event_emitter::callback_t __cb)
{ m_base->listen(__port, "127.0.0.1", __cb); }


void
tcp_server::listen(
    unsigned short            __port,
    std::string               __host,
    event_emitter::callback_t __cb)
{ m_base->listen(__port, __host, __cb); }
