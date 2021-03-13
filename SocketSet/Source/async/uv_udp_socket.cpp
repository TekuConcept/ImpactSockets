/**
 * Created by TekuConcept on March 4, 2021
 */

#include <iostream>
#include <cstring>
#include <algorithm>
#include "async/uv_udp_socket.h"

using namespace impact;

#define V(x) std::cout << x << std::endl


uv_udp_socket::uv_udp_socket(uv_event_loop* __event_loop)
{
    m_event_loop             = (__event_loop);

    m_address.address        = "";
    m_address.family         = address_family::UNSPECIFIED;
    m_address.port           = 0;

    m_remote_address.address = "";
    m_remote_address.family  = address_family::UNSPECIFIED;
    m_remote_address.port    = 0;

    m_hints.ai_family        = PF_INET;
    m_hints.ai_socktype      = SOCK_DGRAM;
    m_hints.ai_protocol      = IPPROTO_UDP;
    m_hints.ai_flags         = 0;

    m_handle = std::make_shared<uv_udp_t>();
    int result = uv_udp_init(m_event_loop->get_loop_handle(), m_handle.get());
    if (result != 0) throw impact_error("unexpected uv error");
    m_handle->data = (void*)this;

    m_fast_events = this;

    m_event_loop->add_child(this);
}


uv_udp_socket::~uv_udp_socket()
{
    m_event_loop->remove_child(this);
    m_event_loop->invoke([this]() { _M_destroy(); }, /*blocking=*/true);

    // free all allocated memory
    for (auto& buffer : m_malloc_buffers)
        free((void*)buffer);
    m_malloc_buffers.clear();
}


udp_address_t
uv_udp_socket::address() const
{ return m_address; }


udp_address_t
uv_udp_socket::remote_address() const
{ return m_remote_address; }


// size_t
// uv_udp_socket::recv_buffer_size() const
// { return m_recv_buffer_size; }


// size_t
// uv_udp_socket::send_buffer_size() const
// { return m_send_buffer_size; }


// void
// uv_udp_socket::recv_buffer_size(size_t __size)
// { m_recv_buffer_size = __size; }


// void
// uv_udp_socket::send_buffer_size(size_t __size)
// { m_send_buffer_size = __size; }


void
uv_udp_socket::add_membership(
    std::string __address,
    std::string __interface)
{
    m_event_loop->invoke([this, __address, __interface]() {
        _M_add_membership(__address, __interface);
    });
}


void
uv_udp_socket::drop_membership(
    std::string __address,
    std::string __interface)
{
    m_event_loop->invoke([this, __address, __interface]() {
        _M_drop_membership(__address, __interface);
    });
}


// void
// uv_udp_socket::add_source_specific_membership(
//     std::string __source_address,
//     std::string __group_address,
//     std::string __interface)
// {
//     m_event_loop->invoke([
//         this,
//         __source_address,
//         __group_address,
//         __interface]()
//     {
//         _M_add_source_specific_membership(
//             __source_address,
//             __group_address,
//             __interface);
//     });
// }


// void
// uv_udp_socket::drop_source_specific_membership(
//     std::string __source_address,
//     std::string __group_address,
//     std::string __interface)
// {
//     m_event_loop->invoke([
//         this,
//         __source_address,
//         __group_address,
//         __interface]()
//     {
//         _M_drop_source_specific_membership(
//             __source_address,
//             __group_address,
//             __interface);
//     });
// }


// void
// uv_udp_socket::connect(
//     unsigned short            __port,
//     std::string               __address,
//     event_emitter::callback_t __cb)
// {
//     event_emitter::once("connect", __cb);
//     m_event_loop->invoke([this, __port, __address]() {
//         _M_connect(__port, __address);
//     });
// }


// void
// uv_udp_socket::disconnect()
// { m_event_loop->invoke([this]() { _M_disconnect(); }); }


void
uv_udp_socket::bind(
    unsigned short            __port,
    std::string               __address,
    event_emitter::callback_t __cb)
{
    m_event_loop->invoke([this, __port, __address, __cb]() {
        _M_bind(__port, __address, __cb);
    });
}


void
uv_udp_socket::close(event_emitter::callback_t __cb)
{
    event_emitter::on("close", __cb);
    m_event_loop->invoke([this]() { _M_destroy(); });
}


void
uv_udp_socket::send(
    std::string               __message,
    size_t                    __offset,
    size_t                    __length,
    unsigned short            __port,
    std::string               __address,
    event_emitter::callback_t __cb)
{
    m_event_loop->invoke([
        this,     __message,
        __offset, __length,
        __port,   __address,
        __cb
    ]() {
        _M_send(
            __message,
            __offset, __length,
            __port,   __address,
            __cb
        );
    });
}


void
uv_udp_socket::pause()
{ m_event_loop->invoke([this]() { _M_pause(); }); }


void
uv_udp_socket::resume()
{ m_event_loop->invoke([this]() { _M_resume(); }); }


void
uv_udp_socket::set_broadcast(bool __flag)
{
    m_event_loop->invoke([this, __flag]() {
        _M_set_broadcast(__flag);
    });
}


void
uv_udp_socket::set_multicast_interface(std::string __interface)
{
    m_event_loop->invoke([this, __interface]() {
        _M_set_multicast_interface(__interface);
    });
}


void
uv_udp_socket::set_multicast_loopback(bool __flag)
{
    m_event_loop->invoke([this, __flag]() {
        _M_set_multicast_loopback(__flag);
    });
}


void
uv_udp_socket::set_multicast_ttl(unsigned char __time_to_live)
{
    m_event_loop->invoke([this, __time_to_live]() {
        _M_set_multicast_ttl(__time_to_live);
    });
}


void
uv_udp_socket::set_ttl(unsigned char __time_to_live)
{
    m_event_loop->invoke([this, __time_to_live]() {
        _M_set_ttl(__time_to_live);
    });
}


void
uv_udp_socket::set_event_observer(udp_socket_observer_interface* __fast_events)
{ m_fast_events = (__fast_events != nullptr) ? __fast_events : this; }


void
uv_udp_socket::send_signal(uv_node_signal_t __op)
{
    switch (__op) {
    case uv_node_signal_t::STOP:
        this->close();
        break;
    }
}


void
uv_udp_socket::on_close()
{ event_emitter::emit("close"); }


void
uv_udp_socket::on_connect()
{ event_emitter::emit("connect"); }


void
uv_udp_socket::on_error(const std::string& __message)
{ event_emitter::emit("error", __message); }


void
uv_udp_socket::on_listening()
{ event_emitter::emit("listening"); }


void
uv_udp_socket::on_message(
    const std::string&   __data,
    const udp_address_t& __address)
{ event_emitter::emit("message", __data, __address); }


void
uv_udp_socket::_M_add_membership(
    const std::string& __address,
    const std::string& __interface)
{
    auto result = uv_udp_set_membership(
        m_handle.get(),
        __address.c_str(),
        __interface.size() > 0 ? __interface.c_str() : nullptr,
        UV_JOIN_GROUP);
    if (result != 0) _M_emit_error_code(__FUNCTION__, result);
}


void
uv_udp_socket::_M_drop_membership(
    const std::string& __address,
    const std::string& __interface)
{
    auto result = uv_udp_set_membership(
        m_handle.get(),
        __address.c_str(),
        __interface.size() > 0 ? __interface.c_str() : nullptr,
        UV_LEAVE_GROUP);
    if (result != 0) _M_emit_error_code(__FUNCTION__, result);
}


// void
// uv_udp_socket::_M_add_source_specific_membership(
//     const std::string& __source_address,
//     const std::string& __group_address,
//     const std::string& __interface)
// {
//     auto result = uv_udp_set_source_membership(
//         m_handle.get(),
//         __group_address.c_str(),
//         __interface.size() > 0 ? __interface.c_str() : nullptr,
//         __source_address.c_str(),
//         UV_JOIN_GROUP);
//     if (result != 0) _M_emit_error_code(__FUNCTION__, result);
// }


// void
// uv_udp_socket::_M_drop_source_specific_membership(
//     const std::string& __source_address,
//     const std::string& __group_address,
//     const std::string& __interface)
// {
//     auto result = uv_udp_set_source_membership(
//         m_handle.get(),
//         __group_address.c_str(),
//         __interface.size() > 0 ? __interface.c_str() : nullptr,
//         __source_address.c_str(),
//         UV_LEAVE_GROUP);
//     if (result != 0) _M_emit_error_code(__FUNCTION__, result);
// }


// void
// uv_udp_socket::_M_connect(
//     unsigned short     __port,
//     const std::string& __address)
// {
//     int result;
//     sockaddr_storage addr;
//     result = _M_info_to_address(__port, __address, (struct sockaddr*)&addr);
//     if (result != 0) goto error;
//     _M_fill_address_info((struct sockaddr*)&addr, &m_remote_address);
//     result = uv_udp_connect(m_handle.get(), (struct sockaddr*)&addr);
//     if (result == 0) return;
//     error:
//     _M_emit_error_code(__FUNCTION__, result);
// }


// void
// uv_udp_socket::_M_disconnect()
// {
//     auto result = uv_udp_connect(m_handle.get(), nullptr);
//     if (result != 0) _M_emit_error_code(__FUNCTION__, result);
//     else {
//         m_remote_address.address = "";
//         m_remote_address.family  = address_family::UNSPECIFIED;
//         m_remote_address.port    = 0;
//     }
// }


void
uv_udp_socket::_M_bind(
    unsigned short            __port,
    const std::string&        __address,
    event_emitter::callback_t __cb)
{
    int result;
    sockaddr_storage addr;

    result = _M_info_to_address(__port, __address, (struct sockaddr*)&addr);
    if (result != 0) goto error;    
    _M_fill_address_info((struct sockaddr*)&addr, &m_address);
    result = uv_udp_bind(
        m_handle.get(),
        (struct sockaddr*)&addr,
        UV_UDP_REUSEADDR);
    if (result != 0) goto error;

    _M_resume();

    if (__cb) __cb({});
    m_fast_events->on_listening();
    return;

    error:
    _M_emit_error_code(__FUNCTION__, result);
}


void
uv_udp_socket::_M_send(
    const std::string&        __message,
    size_t                    __offset,
    size_t                    __length,
    unsigned short            __port,
    const std::string&        __address,
    event_emitter::callback_t __cb)
{
    int result;
    uv_udp_send_t* request;
    sockaddr_storage addr;
    uv_buf_t buffers[1];
    write_context_t* context;

    result = _M_info_to_address(__port, __address, (struct sockaddr*)&addr);
    if (result != 0) goto error;

    context = new write_context_t;
    context->socket = this;
    context->data   = __message.substr(__offset, __length);
    context->cb     = __cb;
    buffers[0].base = &context->data[0];
    buffers[0].len  = context->data.size();

    request = new uv_udp_send_t;
    memset(request, 0, sizeof(uv_udp_send_t));
    request->data = context;

    result = uv_udp_send(
        request,
        m_handle.get(),
        buffers,
        /*size=*/1,
        (struct sockaddr*)&addr,
        _S_on_send_callback);

    if (result == 0) return;

    delete request;
    delete context;

    error:
    _M_emit_error_code(__FUNCTION__, result);
}


void
uv_udp_socket::_M_pause()
{
    auto result = uv_udp_recv_stop(m_handle.get());
    if (result != 0) _M_emit_error_code(__FUNCTION__, result);
}


void
uv_udp_socket::_M_resume()
{
    auto result = uv_udp_recv_start(
        m_handle.get(),
        _S_alloc_buffer_callback,
        _S_on_recv_callback);
    if (result != 0) _M_emit_error_code(__FUNCTION__, result);
}


void
uv_udp_socket::_M_set_broadcast(bool __flag)
{
    auto result = uv_udp_set_broadcast(m_handle.get(), (int)__flag);
    if (result != 0) _M_emit_error_code(__FUNCTION__, result);
}


void
uv_udp_socket::_M_set_multicast_interface(const std::string& __interface)
{
    auto result = uv_udp_set_multicast_interface(
        m_handle.get(), __interface.c_str());
    if (result != 0) _M_emit_error_code(__FUNCTION__, result);
}


void
uv_udp_socket::_M_set_multicast_loopback(bool __flag)
{
    auto result = uv_udp_set_multicast_loop(m_handle.get(), (int)__flag);
    if (result != 0) _M_emit_error_code(__FUNCTION__, result);
}


void
uv_udp_socket::_M_set_multicast_ttl(unsigned char __time_to_live)
{
    auto result = uv_udp_set_multicast_ttl(m_handle.get(), __time_to_live);
    if (result != 0) _M_emit_error_code(__FUNCTION__, result);
}


void
uv_udp_socket::_M_set_ttl(unsigned char __time_to_live)
{
    auto result = uv_udp_set_ttl(m_handle.get(), __time_to_live);
    if (result != 0) _M_emit_error_code(__FUNCTION__, result);
}


void
uv_udp_socket::_M_destroy()
{
    if (!uv_is_closing((uv_handle_t*)m_handle.get()))
        uv_close((uv_handle_t*)m_handle.get(), _S_on_close_callback);
}


void
uv_udp_socket::_M_emit_error_code(
    std::string __message,
    int         __code)
{
    std::string message = __message;
    message += std::string(": ");
    message += std::to_string(__code) + std::string(" ");
    message += uv_strerror(__code);
    m_fast_events->on_error(message);
}


void
uv_udp_socket::_M_fill_address_info(
    const struct sockaddr* __address,
    udp_address_t*         __dest)
{
    void* address_data;
    __dest->family = (address_family)__address->sa_family;

    switch (__dest->family) {
    case address_family::INET: {
        __dest->address.resize(INET_ADDRSTRLEN);
        __dest->port = ntohs(((sockaddr_in*)__address)->sin_port);
        address_data   = &((sockaddr_in*)__address)->sin_addr;
    } break;
    case address_family::INET6: {
        __dest->address.resize(INET6_ADDRSTRLEN);
        __dest->port = ntohs(((sockaddr_in6*)__address)->sin6_port);
        address_data   = &((sockaddr_in6*)__address)->sin6_addr;
    } break;
    default: /* unsupported */ break;
    }

    auto status2 = inet_ntop(
        __address->sa_family,
        address_data,
        &__dest->address[0],
        __dest->address.size());
    if (status2 == nullptr)
        m_fast_events->on_error("unexpected address error");
}


int
uv_udp_socket::_M_info_to_address(
    unsigned short     __port,
    const std::string& __address,
    struct sockaddr*   __sockaddr)
{
    int result = uv_ip4_addr(__address.c_str(),
        __port, (sockaddr_in*)__sockaddr);
    if (result) {
        result = uv_ip6_addr(__address.c_str(),
            __port, (sockaddr_in6*)__sockaddr);
        if (result) return result;
        else m_address.family = address_family::INET6;
    }
    else m_address.family = address_family::INET;
    return 0;
}


char*
uv_udp_socket::_M_alloc_buffer(size_t __size)
{
    char* buffer = (char*)malloc(__size);
    m_malloc_buffers.push_back(buffer);
    return buffer;
}


void
uv_udp_socket::_M_free_buffer(char* __buffer)
{
    auto source = std::find(
        m_malloc_buffers.begin(),
        m_malloc_buffers.end(),
        __buffer);
    if (source == m_malloc_buffers.end())
        m_fast_events->on_error("pausible memory leak");
    else {
        free((void*)__buffer);
        m_malloc_buffers.erase(source);
    }
}


void
uv_udp_socket::_S_on_close_callback(uv_handle_t* __handle)
{
    auto* socket = reinterpret_cast<uv_udp_socket*>(__handle->data);
    socket->m_fast_events->on_close();
}


void
uv_udp_socket::_S_on_send_callback(
    uv_udp_send_t* __request,
    int            __status)
{
    auto result = __status;
    auto* context =
        reinterpret_cast<uv_udp_socket::write_context_t*>(__request->data);
    if (result < 0)
        context->socket->_M_emit_error_code("send", result);
    else {
        if (context->cb)
            context->cb({});
    }
    delete context;
    delete __request;
}


void
uv_udp_socket::_S_on_recv_callback(
    uv_udp_t*              __handle,
    ssize_t                __nread,
    const uv_buf_t*        __buffer,
    const struct sockaddr* __address,
    unsigned             /*__flags*/)
{
    auto* socket = reinterpret_cast<uv_udp_socket*>(__handle->data);
    if (__nread >= 0) {
        std::string data;
        udp_address_t address;
        if (__buffer->base != NULL) {
            data = std::string(__buffer->base, sizeof(char) * __nread);
            socket->_M_free_buffer(__buffer->base);
        }
        // else empty datagram: return an empty string
        if (__address != NULL)
            socket->_M_fill_address_info(__address, &address);
        else {
            address.address = "";
            address.family  = address_family::UNSPECIFIED;
            address.port    = 0;
        }
        socket->m_fast_events->on_message(data, address);
    }
    else socket->_M_emit_error_code("read", __nread);
}


void
uv_udp_socket::_S_alloc_buffer_callback(
    uv_handle_t* __handle,
    size_t       __suggested_size,
    uv_buf_t*    __buffer)
{
    auto* client = reinterpret_cast<uv_udp_socket*>(__handle->data);
    __buffer->base = client->_M_alloc_buffer(__suggested_size);
    __buffer->len  = __suggested_size;
}
