/**
 * Created by TekuConcept on January 19, 2021
 */

#include <iostream>
#include "async/uv_tcp_server.h"
#include "utils/impact_error.h"

using namespace impact;
#define VERBOSE(x) std::cout << x << std::endl

namespace impact {

    void
    uv_tcp_server_on_close(uv_handle_t* __handle)
    {
        auto* server =
            reinterpret_cast<uv_tcp_server*>(__handle->data);
        server->m_is_listening = false;
        server->emit("close");
    }

    void
    uv_tcp_server_on_connection(
        uv_stream_t* __server,
        int          __status)
    {
        auto result = __status;
        auto* server =
            reinterpret_cast<uv_tcp_server*>(__server->data);
        if (result < 0) goto error;

        {
            uv_tcp_t* client = new uv_tcp_t();
            uv_tcp_init(&server->m_event_loop->loop, client);
            if (uv_accept(__server, (uv_stream_t*)client) == 0) {
            //     if (server->m_client_count.load() >= server->m_max_connections)
            //         uv_close((uv_handle_t*)&client, NULL);
            //     else {
            //         // server->m_clients->count++; // decremented in uv_tcp_socket

            //         // TODO
            //         // setup async callbacks
            //         // wrap raw connection
            //         // event_emitter::emit("connection", connection);

            //         std::cout << "new connection accepted" << std::endl;
            //         uv_close((uv_handle_t*)&client, NULL);
            //     }
            }
            // else uv_close((uv_handle_t*)&client, NULL);

            uv_close((uv_handle_t*)client,
                [](uv_handle_t* handle) { delete handle; });
        }

        VERBOSE("connection received event");
        return;

        error:
        std::string message = "connection error: ";
        message += std::to_string(result) + std::string(" ");
        message += uv_strerror(result);
        server->emit("error", message);
    }


    void
    uv_tcp_server_on_path_resolved(
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


    void
    uv_tcp_server_async_callback(uv_async_t* __async)
    {
        auto* server =
            reinterpret_cast<uv_tcp_server*>(__async->data);
        std::vector<struct uv_tcp_server::async_request_t> queue;
        if (std::this_thread::get_id() != server->m_event_loop->isolate)
            return;

        uv_rwlock_wrlock(&server->m_lock);
        queue = std::move(server->m_requests);
        uv_rwlock_wrunlock(&server->m_lock);

        using request_type = uv_tcp_server::request_type;
        for (auto& request : queue) {
            switch (request.type) {
            case request_type::CLOSE:
                server->_M_close();
                break;
            case request_type::LISTEN1:
                server->_M_listen(request.port, request.host);
                break;
            case request_type::LISTEN2:
                server->_M_listen(request.host);
                break;
            }
            if (request.promise) request.promise->set_value();
        }
    }

}


uv_tcp_server::async_request_t::async_request_t()
: type(request_type::CLOSE),
  host(""),
  port(0),
  promise(nullptr)
{ }


uv_tcp_server::uv_tcp_server(
    std::shared_ptr<struct uv_event_loop::context_t> __event_loop)
: m_event_loop(__event_loop),
  m_is_listening(false),
  m_max_connections((size_t)-1),
  m_client_count(0)
{
    m_address.address   = "";
    m_address.family    = address_family::UNSPECIFIED;
    m_address.port      = 0;

    m_hints.ai_family   = PF_INET;
    m_hints.ai_socktype = SOCK_STREAM;
    m_hints.ai_protocol = IPPROTO_TCP;
    m_hints.ai_flags    = 0;

    int result = uv_tcp_init(&m_event_loop->loop, &m_handle);
    if (result != 0) throw impact_error("unexpected uv error");
    m_handle.data = (void*)this;

    uv_rwlock_init(&m_lock);
    uv_async_init(
        &m_event_loop->loop,
        &m_async_handle,
        uv_tcp_server_async_callback);
    m_async_handle.data = (void*)this;
}


uv_tcp_server::~uv_tcp_server()
{
    if (!uv_is_closing((uv_handle_t*)&m_handle)) {
        if (std::this_thread::get_id() == m_event_loop->isolate)
            _M_close();
        else _M_close_async(/*blocking=*/true);
    }
    uv_close(reinterpret_cast<uv_handle_t*>(&m_async_handle), nullptr);
    uv_rwlock_destroy(&m_lock);
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
    event_emitter::on("close", __cb);
    if (std::this_thread::get_id() == m_event_loop->isolate)
        _M_close();
    else _M_close_async();
}


void
uv_tcp_server::listen(
    std::string               __path,
    event_emitter::callback_t __cb)
{
    event_emitter::on("listening", __cb);
    if (std::this_thread::get_id() == m_event_loop->isolate)
        _M_listen(__path);
    else _M_listen_async(__path);
}


void
uv_tcp_server::listen(
    unsigned short            __port,
    event_emitter::callback_t __cb)
{ listen(__port, "127.0.0.1", __cb); }


void
uv_tcp_server::listen(
    unsigned short            __port,
    std::string               __host,
    event_emitter::callback_t __cb)
{
    event_emitter::on("listening", __cb);
    if (std::this_thread::get_id() == m_event_loop->isolate)
        _M_listen(__port, __host);
    else _M_listen_async(__port, __host);
}


void
uv_tcp_server::_M_close()
{ uv_close((uv_handle_t*)&m_handle, uv_tcp_server_on_close); }


void
uv_tcp_server::_M_close_async(bool __blocking)
{
    struct async_request_t request;
    request.type = request_type::CLOSE;

    std::promise<void> p;
    std::future<void> f;
    if (__blocking) {
        request.promise = &p;
        f = p.get_future();
    }

    uv_rwlock_wrlock(&m_lock);
    m_requests.push_back(request);
    uv_rwlock_wrunlock(&m_lock);
    uv_async_send(&m_async_handle);

    if (__blocking) try { f.get(); } catch (...) { /* ignore */ }
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
        uv_tcp_server_on_connection);
    if (result) goto error;

    m_is_listening = true;
    event_emitter::emit("listening");
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
    result = uv_getaddrinfo(&m_event_loop->loop, resolver,
        uv_tcp_server_on_path_resolved, __path.c_str(), nullptr, &m_hints);
    if (result) goto error;
    return;

    error:
    _M_emit_listen_error(result);
}


void
uv_tcp_server::_M_listen_async(
    unsigned short __port,
    std::string    __host)
{
    struct async_request_t request;
    request.type = request_type::LISTEN1;
    request.port = __port;
    request.host = __host;

    uv_rwlock_wrlock(&m_lock);
    m_requests.push_back(request);
    uv_rwlock_wrunlock(&m_lock);
    uv_async_send(&m_async_handle);
}


void
uv_tcp_server::_M_listen_async(std::string __path)
{
    struct async_request_t request;
    request.type = request_type::LISTEN2;
    request.host = __path;

    uv_rwlock_wrlock(&m_lock);
    m_requests.push_back(request);
    uv_rwlock_wrunlock(&m_lock);
    uv_async_send(&m_async_handle);
}


void
uv_tcp_server::_M_emit_listen_error(int __status)
{
    std::string message = "listen error: ";
    message += std::to_string(__status) + std::string(" ");
    message += uv_strerror(__status);
    event_emitter::emit("error", message);
}
