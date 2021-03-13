/**
 * Created by TekuConcept on January 19, 2021
 */

#include <iostream>
#include <cstring>
#include <memory>
#include <algorithm>
#include "async/uv_tcp_server.h"
#include "async/uv_tcp_client.h"
#include "utils/impact_error.h"

using namespace impact;

#define V(x) std::cout << x << std::endl


uv_tcp_server::uv_tcp_server(uv_event_loop* __event_loop)
: m_event_loop(__event_loop),
  m_is_listening(false),
  m_max_connections((size_t)-1)
{
    m_address.address   = "";
    m_address.family    = address_family::UNSPECIFIED;
    m_address.port      = 0;

    m_hints.ai_family   = PF_INET;
    m_hints.ai_socktype = SOCK_STREAM;
    m_hints.ai_protocol = IPPROTO_TCP;
    m_hints.ai_flags    = 0;

    int result = uv_tcp_init(m_event_loop->get_loop_handle(), &m_handle);
    if (result != 0) throw impact_error("unexpected uv error");
    m_handle.data = (void*)this;

    m_fast_events = this;
    m_event_loop->add_child(this);
}


uv_tcp_server::~uv_tcp_server()
{
    m_event_loop->remove_child(this);
    m_event_loop->invoke([this]() { _M_close(); }, /*blocking=*/true);
}


tcp_address_t
uv_tcp_server::address() const
{ return m_address; }


bool
uv_tcp_server::listening() const
{ return m_is_listening; }


size_t
uv_tcp_server::max_connections() const
{ return m_max_connections; }


void
uv_tcp_server::max_connections(size_t value)
{ m_max_connections = value; }


void
uv_tcp_server::close(event_emitter::callback_t __cb)
{
    event_emitter::once("close", __cb);
    m_event_loop->invoke([this]() { _M_close(); });
}


void
uv_tcp_server::listen(
    std::string               __path,
    event_emitter::callback_t __cb)
{
    event_emitter::once("listening", __cb);
    m_event_loop->invoke([this, __path]() { _M_listen(__path); });
}


void
uv_tcp_server::listen(
    unsigned short            __port,
    std::string               __host,
    event_emitter::callback_t __cb)
{
    event_emitter::once("listening", __cb);
    m_event_loop->invoke([this, __port, __host]() {
        _M_listen(__port, __host);
    });
}


void
uv_tcp_server::set_event_observer(
    tcp_server_observer_interface* __fast_events)
{ m_fast_events = __fast_events != nullptr ? __fast_events : this; }


void
uv_tcp_server::send_signal(uv_node_signal_t __op)
{
    switch (__op) {
    case uv_node_signal_t::STOP:
        this->close();
        break;
    }
}


void
uv_tcp_server::on_close()
{ event_emitter::emit("close"); }


void
uv_tcp_server::on_connection(const tcp_client_t& __connection)
{ event_emitter::emit("connection", __connection); }


void
uv_tcp_server::on_error(const std::string& __message)
{ event_emitter::emit("error", __message); }


void
uv_tcp_server::on_listening()
{ event_emitter::emit("listening"); }


void
uv_tcp_server::_M_close()
{
    if (!uv_is_closing((uv_handle_t*)&m_handle))
        uv_close((uv_handle_t*)&m_handle, _S_on_close_callback);
}


void
uv_tcp_server::_M_listen(
    unsigned short __port,
    std::string    __host)
{
    int result;
    union {
        sockaddr_in ipv4;
        sockaddr_in6 ipv6;
    } addr;

    result = uv_ip4_addr(__host.c_str(), __port, &addr.ipv4);
    if (result) {
        result = uv_ip6_addr(__host.c_str(), __port, &addr.ipv6);
        if (result) goto error;
        else m_address.family = address_family::INET6;
    }
    else m_address.family = address_family::INET;

    _M_listen((struct sockaddr*)&addr);
    return;

    error:
    _M_emit_listen_error(result);
}


void
uv_tcp_server::_M_listen(std::string __path)
{
    int result;
    uv_getaddrinfo_t* resolver;

    resolver = new uv_getaddrinfo_t();
    resolver->data = (void*)this;
    result = uv_getaddrinfo(m_event_loop->get_loop_handle(), resolver,
        _S_on_path_resolved_callback, __path.c_str(), nullptr, &m_hints);
    if (result) goto error;
    return;

    error:
    _M_emit_listen_error(result);
}


void
uv_tcp_server::_M_listen(const struct sockaddr* __addr)
{
    int result = uv_tcp_bind(&m_handle, __addr, 0);    
    if (result) goto error;

    if (m_address.family == address_family::INET) {
        char str[INET_ADDRSTRLEN];
        struct sockaddr_in* pv4addr = (struct sockaddr_in*)&__addr;
        result = uv_ip4_name(pv4addr, str, INET_ADDRSTRLEN);
        m_address.address = std::string(str);
        m_address.port    = pv4addr->sin_port;
        m_address.family  = address_family::INET;
    }
    else {
        char str[INET6_ADDRSTRLEN];
        struct sockaddr_in6* pv6addr = (struct sockaddr_in6*)&__addr;
        result = uv_ip6_name(pv6addr, str, INET6_ADDRSTRLEN);
        m_address.address = std::string(str);
        m_address.port    = pv6addr->sin6_port;
        m_address.family  = address_family::INET6;
    }

    m_handle.data = (void*)this;

    result = uv_listen(
        (uv_stream_t*)&m_handle,
        128/*backlog*/,
        _S_on_connection_callback);
    if (result) goto error;

    m_is_listening = true;
    m_fast_events->on_listening();
    return;

    error:
    _M_emit_listen_error(result);
}


void
uv_tcp_server::_M_emit_listen_error(int __status)
{
    std::string message = "listen error: ";
    message += std::to_string(__status) + std::string(" ");
    message += uv_strerror(__status);
    m_fast_events->on_error(message);
}


void
uv_tcp_server::_M_add_client_reference(uv_tcp_client* __connection)
{
    std::lock_guard<std::mutex> lock(m_list_mtx);
    m_connection_list.push_back(__connection);
}


void
uv_tcp_server::_M_remove_client_reference(uv_tcp_client* __connection)
{
    std::lock_guard<std::mutex> lock(m_list_mtx);
    auto token = std::find(
        m_connection_list.begin(),
        m_connection_list.end(),
        __connection);
    if (token != m_connection_list.end())
        m_connection_list.erase(token);
}


size_t
uv_tcp_server::_M_client_reference_count()
{
    std::lock_guard<std::mutex> lock(m_list_mtx);
    return m_connection_list.size();
}


void
uv_tcp_server::_S_on_close_callback(uv_handle_t* __handle)
{
    auto* server =
        reinterpret_cast<uv_tcp_server*>(__handle->data);
    server->m_is_listening = false;
    server->m_fast_events->on_close();
}


void
uv_tcp_server::_S_on_connection_callback(
    uv_stream_t* __server,
    int          __status)
{
    auto result = __status;
    auto* server =
        reinterpret_cast<uv_tcp_server*>(__server->data);

    if (result < 0) {
        std::string message = "connection error: ";
        message += std::to_string(result) + std::string(" ");
        message += uv_strerror(result);
        server->m_fast_events->on_error(message);
        return;
    }

    uv_tcp_t* socket = new uv_tcp_t();
    uv_tcp_init(server->m_event_loop->get_loop_handle(), socket);

    if (uv_accept(__server, (uv_stream_t*)socket) == 0) {
        if (server->_M_client_reference_count() >= server->m_max_connections)
            uv_close((uv_handle_t*)socket,
                [](uv_handle_t* handle) { delete handle; });
        else {
            std::shared_ptr<uv_tcp_client> client =
                std::shared_ptr<uv_tcp_client>(new uv_tcp_client(
                    server->m_event_loop,
                    server,
                    std::shared_ptr<uv_tcp_t>(socket)
                ));
            // removed by client; only store the pointer and not
            // the shared pointer so the shared pointer may be
            // properly decremented on closure
            server->_M_add_client_reference(client.get());
            server->m_fast_events->on_connection(client);
        }
    }
    else uv_close((uv_handle_t*)socket,
        [](uv_handle_t* handle) { delete handle; });
}


void
uv_tcp_server::_S_on_path_resolved_callback(
    uv_getaddrinfo_t* __resolver,
    int               __status,
    struct addrinfo*  __info)
{
    auto result = __status;
    auto* server =
        reinterpret_cast<uv_tcp_server*>(__resolver->data);
    if (result < 0) goto error;
    server->_M_listen(__info->ai_addr);
    delete __resolver;
    uv_freeaddrinfo(__info);
    return;

    error:
    server->_M_emit_listen_error(result);
    delete __resolver;
}
