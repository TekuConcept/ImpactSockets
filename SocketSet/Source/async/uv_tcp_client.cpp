/**
 * Created by TekuConcept on February 11, 2021
 */

#include <iostream>
#include <cstring>
#include <algorithm>
#include "async/uv_tcp_client.h"
#include "async/uv_tcp_server.h"
#include "utils/impact_error.h"

using namespace impact;

#define V(x) std::cout << x << std::endl


uv_tcp_client::uv_tcp_client(uv_event_loop* __event_loop)
{
    _M_init(__event_loop);

    m_handle = std::make_shared<uv_tcp_t>();
    int result = uv_tcp_init(m_event_loop->get_loop_handle(), m_handle.get());
    if (result != 0) throw impact_error("unexpected uv error");
    m_handle->data = (void*)this;
}


uv_tcp_client::uv_tcp_client(
    uv_event_loop*            __event_loop,
    uv_tcp_server*            __parent_server,
    std::shared_ptr<uv_tcp_t> __base_connection)
: m_server(__parent_server),
  m_handle(__base_connection)
{
    // called by uv_tcp_server
    _M_init(__event_loop);
    m_handle->data = (void*)this;
    m_stream       = (uv_stream_t*)m_handle.get();
    m_ready_state  = uv_tcp_client::ready_state_t::OPEN;
    _M_resume();
}


uv_tcp_client::~uv_tcp_client()
{
    m_event_loop->remove_child(this);

    if (!uv_is_closing((uv_handle_t*)m_handle.get()))
        m_event_loop->invoke([this]() { _M_destroy(""); }, /*blocking=*/true);

    // free all allocated memory
    for (auto& buffer : m_malloc_buffers)
        free((void*)buffer);
    m_malloc_buffers.clear();
}


void
uv_tcp_client::_M_init(uv_event_loop* __event_loop)
{
    m_event_loop             = (__event_loop);
    m_bytes_read             = (0);
    m_bytes_written          = (0);
    m_timeout                = (0);
    m_has_timeout            = (false);
    m_encoding               = ("utf8");
    m_ready_state            = (ready_state_t::PENDING);
    m_server                 = (nullptr);

    m_address.address        = "";
    m_address.family         = address_family::UNSPECIFIED;
    m_address.port           = 0;

    m_local_address.address  = "";
    m_local_address.family   = address_family::UNSPECIFIED;
    m_local_address.port     = 0;

    m_remote_address.address = "";
    m_remote_address.family  = address_family::UNSPECIFIED;
    m_remote_address.port    = 0;

    m_hints.ai_family        = PF_INET;
    m_hints.ai_socktype      = SOCK_STREAM;
    m_hints.ai_protocol      = IPPROTO_TCP;
    m_hints.ai_flags         = AI_CANONNAME;

    m_fast_events            = this;

    m_event_loop->add_child(this);
}


tcp_address_t
uv_tcp_client::address() const
{ return m_address; }


size_t
uv_tcp_client::bytes_read() const
{ return m_bytes_read; }


size_t
uv_tcp_client::bytes_written() const
{ return m_bytes_written; }


bool
uv_tcp_client::connecting() const
{ return m_ready_state == ready_state_t::OPENING; }


bool
uv_tcp_client::destroyed() const
{ return m_ready_state == ready_state_t::DESTROYED; }


std::string
uv_tcp_client::local_address() const
{ return m_local_address.address; }


unsigned short
uv_tcp_client::local_port() const
{ return m_local_address.port; }


bool
uv_tcp_client::pending() const
{ return m_ready_state == ready_state_t::PENDING; }


std::string
uv_tcp_client::remote_address() const
{ return m_remote_address.address; }


address_family
uv_tcp_client::remote_family() const
{ return m_remote_address.family; }


unsigned short
uv_tcp_client::remote_port() const
{ return m_remote_address.port; }


size_t
uv_tcp_client::timeout() const
{ return m_timeout; }


tcp_client_interface::ready_state_t
uv_tcp_client::ready_state() const
{ return m_ready_state.load(); }


tcp_client_interface*
uv_tcp_client::connect(
    std::string               __path,
    event_emitter::callback_t __cb)
{
    event_emitter::once("connect", __cb);
    m_event_loop->invoke([this, __path]() { _M_connect(__path); });
    return this;
}


tcp_client_interface*
uv_tcp_client::connect(
    unsigned short            __port,
    std::string               __host,
    event_emitter::callback_t __cb)
{
    event_emitter::once("connect", __cb);
    m_event_loop->invoke([this, __port, __host]() {
        _M_connect(__port, __host);
    });
    return this;
}


tcp_client_interface*
uv_tcp_client::destroy(std::string __error)
{
    // if (m_ready_state == ready_state_t::DESTROYED)
    //     return this;
    m_event_loop->invoke([this, __error]() { _M_destroy(__error); });
    return this;
}


tcp_client_interface*
uv_tcp_client::end(
    std::string               __data,
    std::string               __encoding,
    event_emitter::callback_t __cb)
{
    if (m_ready_state != ready_state_t::OPEN &&
        m_ready_state != ready_state_t::WRITE_ONLY)
        return this;
    event_emitter::once("end", __cb);
    m_event_loop->invoke([this, __data, __encoding]() {
        _M_end(__data, __encoding);
    });
    return this;
}


tcp_client_interface*
uv_tcp_client::pause()
{
    m_event_loop->invoke([this]() { _M_pause(); });
    return this;
}


tcp_client_interface*
uv_tcp_client::resume()
{
    m_event_loop->invoke([this]() { _M_resume(); });
    return this;
}


tcp_client_interface*
uv_tcp_client::set_encoding(std::string /*__encoding*/)
{ /* always use UTF8 for now */ return this; }


tcp_client_interface*
uv_tcp_client::set_keep_alive(
    bool         __enable,
    unsigned int __initial_delay)
{
    m_event_loop->invoke([this, __enable, __initial_delay]() {
        _M_set_keepalive(__enable, __initial_delay);
    });
    return this;
}


tcp_client_interface*
uv_tcp_client::set_no_delay(bool __no_delay)
{
    m_event_loop->invoke([this, __no_delay]() {
        _M_set_no_delay(__no_delay);
    });
    return this;
}


tcp_client_interface*
uv_tcp_client::set_timeout(
    unsigned int              __timeout,
    event_emitter::callback_t __cb)
{
    event_emitter::once("timeout", __cb);
    m_event_loop->invoke([this, __timeout]() {
        _M_set_timeout(__timeout);
    });
    return this;
}


bool
uv_tcp_client::write(
    std::string               __data,
    std::string               __encoding,
    event_emitter::callback_t __cb)
{
    if (m_ready_state != ready_state_t::OPEN &&
        m_ready_state != ready_state_t::WRITE_ONLY) {
        m_fast_events->on_error("soc not open");
        return false;
    }

    bool result;
    m_event_loop->invoke([&]() {
        result = _M_write(__data, __encoding, __cb);
    }, /*blocking=*/true);
    return result;
}


void
uv_tcp_client::set_event_observer(tcp_client_observer_interface* __fast_events)
{ m_fast_events = (__fast_events != nullptr) ? __fast_events : this; }


void
uv_tcp_client::send_signal(uv_node_signal_t __op)
{
    switch (__op) {
    case uv_node_signal_t::STOP:
        this->destroy();
        break;
    }
}


void
uv_tcp_client::on_close(bool __transmission_error)
{ event_emitter::emit("close", __transmission_error); }


void
uv_tcp_client::on_connect()
{ event_emitter::emit("connect"); }


void
uv_tcp_client::on_data(std::string& __data)
{ event_emitter::emit("data", __data); }


void
uv_tcp_client::on_end()
{ event_emitter::emit("end"); }


void
uv_tcp_client::on_error(const std::string& __message)
{ event_emitter::emit("error", __message); }


void
uv_tcp_client::on_lookup(
    std::string& __error,
    std::string& __address,
    address_family     __family,
    std::string& __host)
{ event_emitter::emit("lookup", __error, __address, __family, __host); }


void
uv_tcp_client::on_ready()
{ event_emitter::emit("ready"); }


void
uv_tcp_client::on_timeout()
{ event_emitter::emit("timeout"); }


void
uv_tcp_client::_M_set_keepalive(
    bool         __enable,
    unsigned int __delay)
{
    int result = uv_tcp_keepalive(m_handle.get(), __enable, __delay);
    if (result != 0) _M_emit_error_code(__FUNCTION__, result);
}


void
uv_tcp_client::_M_set_no_delay(bool __enable)
{
    int result = uv_tcp_nodelay(m_handle.get(), __enable);
    if (result != 0) _M_emit_error_code(__FUNCTION__, result);
}


void
uv_tcp_client::_M_connect(std::string __path)
{
    int result;
    uv_getaddrinfo_t* resolver;
    m_ready_state = ready_state_t::OPENING;

    resolver = new uv_getaddrinfo_t();
    resolver->data = (void*)this;
    result = uv_getaddrinfo(m_event_loop->get_loop_handle(), resolver,
        _S_on_path_resolved_callback, __path.c_str(), nullptr, &m_hints);
    if (result != 0) goto error;
    return;

    error:
    _M_emit_error_code(__FUNCTION__, result);
}


void
uv_tcp_client::_M_connect(
    unsigned short __port,
    std::string    __host)
{
    int result;
    union {
        sockaddr_in ipv4;
        sockaddr_in6 ipv6;
    } addr;
    m_ready_state = ready_state_t::OPENING;

    result = uv_ip4_addr(__host.c_str(), __port, &addr.ipv4);
    if (result) {
        result = uv_ip6_addr(__host.c_str(), __port, &addr.ipv6);
        if (result) goto error;
        else m_address.family = address_family::INET6;
    }
    else m_address.family = address_family::INET;

    _M_connect((struct sockaddr*)&addr);
    return;

    error:
    _M_emit_error_code(__FUNCTION__, result);
}


void
uv_tcp_client::_M_connect(const struct sockaddr* __address)
{
    _M_fill_address_info(__address, &m_address);
    uv_connect_t* connect_handle = new uv_connect_t();
    connect_handle->data = (void*)this;
    auto result = uv_tcp_connect(
        connect_handle,
        m_handle.get(),
        __address,
        _S_on_connect_callback);
    if (result != 0) _M_emit_error_code(__FUNCTION__, result);
}


void
uv_tcp_client::_M_pause()
{
    auto result = uv_read_stop(m_stream);
    if (result != 0) _M_emit_error_code(__FUNCTION__, result);
}


void
uv_tcp_client::_M_resume()
{
    auto result = uv_read_start(
        m_stream,
        _S_alloc_buffer_callback,
        _S_on_data_callback);
    if (result != 0) _M_emit_error_code(__FUNCTION__, result);
}


bool
uv_tcp_client::_M_write(
    std::string               __data,
    std::string             /*__encoding*/,
    event_emitter::callback_t __cb)
{
    if (__data.size() == 0) {
        if (__cb) __cb({});
        return true;
    }

    uv_buf_t buffers[1];
    write_context_t* context = new write_context_t;
    context->client = this;
    context->data   = __data;
    context->cb     = __cb;
    buffers[0].base = &context->data[0];
    buffers[0].len  = context->data.size();

    uv_write_t* request = new uv_write_t;
    memset(request, 0, sizeof(uv_write_t));
    request->data = context;

    auto result = uv_write(
        request,
        m_stream,
        buffers,
        /*size=*/1,
        _S_on_write_callback);
    if (result != 0) {
        delete request;
        delete context;
        _M_emit_error_code(__FUNCTION__, result);
        return false;
    }
    else return true;
}


void
uv_tcp_client::_M_set_timeout(unsigned int __timeout)
{
    if (__timeout == 0) {
        if (m_has_timeout)
            m_event_loop->clear_timeout(m_timeout_handle);
        m_has_timeout = false;
        m_timeout = 0;
    }
    else {
        m_has_timeout = true;
        m_timeout = __timeout;
        if (m_has_timeout)
            m_event_loop->clear_timeout(m_timeout_handle);
        m_timeout_handle = m_event_loop->set_timeout([&]() {
            m_fast_events->on_timeout();
        }, m_timeout);
    }
}


void
uv_tcp_client::_M_end(
    std::string __data,
    std::string __encoding)
{
    _M_write(__data, __encoding, nullptr);
    uv_shutdown_t* request = new uv_shutdown_t;
    memset(request, 0, sizeof(uv_shutdown_t));
    request->data = this;
    uv_shutdown(request, m_stream, _S_on_shutdown_callback);
}


void
uv_tcp_client::_M_destroy(std::string __error)
{
    // auto result = uv_tcp_close_reset(m_handle.get(), uv_tcp_client_on_close);
    // if (result != 0) _M_emit_error_code("error", result);

    if (!uv_is_closing((uv_handle_t*)m_handle.get()))
        uv_close((uv_handle_t*)m_handle.get(), _S_on_close_callback);

    if (__error.size() > 0)
        m_fast_events->on_error(__error);
}


void
uv_tcp_client::_M_emit_error_code(
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
uv_tcp_client::_M_update_addresses()
{
    typedef int (*func_t)(const uv_tcp_t*, sockaddr*, int*);
    std::vector<std::pair<tcp_address_t*, func_t>> items{
        { &m_local_address,  uv_tcp_getsockname },
        { &m_remote_address, uv_tcp_getpeername }
    };

    for (auto& item : items) {
        struct sockaddr_storage address;
        int size = sizeof(address);

        auto result = item.second(m_handle.get(), (sockaddr*)&address, &size);
        if (result != 0) {
            m_fast_events->on_error("failed to update addresses");
            return;
        }

        _M_fill_address_info((struct sockaddr*)&address, item.first);
    }
}


void
uv_tcp_client::_M_fill_address_info(
    const struct sockaddr* __address,
    tcp_address_t*         __dest)
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


void
uv_tcp_client::_M_on_close()
{
    auto old_state = m_ready_state.load();
    m_ready_state = ready_state_t::DESTROYED;
    if (old_state != ready_state_t::DESTROYED) {
        if (m_server != nullptr)
            m_server->_M_remove_client_reference(this);
        m_fast_events->on_close("");
    }
}


char*
uv_tcp_client::_M_alloc_buffer(size_t __size)
{
    char* buffer = (char*)malloc(__size);
    m_malloc_buffers.push_back(buffer);
    return buffer;
}


void
uv_tcp_client::_M_free_buffer(char* __buffer)
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
uv_tcp_client::_S_on_path_resolved_callback(
    uv_getaddrinfo_t* __resolver,
    int               __status,
    struct addrinfo*  __info)
{
    auto result = __status;
    auto* server =
        reinterpret_cast<uv_tcp_client*>(__resolver->data);
    tcp_address_t hr_address;
    std::string error, host;
    if (result < 0) {
        error = uv_strerror(result);
        server->_M_emit_error_code(__FUNCTION__, result);
    }
    else {
        server->_M_fill_address_info(__info->ai_addr, &hr_address);
        if (__info->ai_canonname)
            host = std::string(__info->ai_canonname);
        server->_M_connect(__info->ai_addr);
        uv_freeaddrinfo(__info);
    }
    server->m_fast_events->on_lookup(
        error,
        hr_address.address,
        hr_address.family,
        host
    );
    delete __resolver;
}


void
uv_tcp_client::_S_on_connect_callback(
    uv_connect_t* __request,
    int           __status)
{
    auto result = __status;
    auto* client =
        reinterpret_cast<uv_tcp_client*>(__request->data);
    if (result < 0)
        client->_M_emit_error_code("connect", result);
    else {
        client->m_stream = __request->handle;
        client->m_stream->data = __request->data;
        client->_M_update_addresses();
        client->m_ready_state = uv_tcp_client::ready_state_t::OPEN;
        client->_M_resume();
        client->m_fast_events->on_connect();
        client->m_fast_events->on_ready();
    }
    delete __request;
}


void
uv_tcp_client::_S_alloc_buffer_callback(
    uv_handle_t* __handle,
    size_t       __suggested_size,
    uv_buf_t*    __buffer)
{
    auto* client = reinterpret_cast<uv_tcp_client*>(__handle->data);
    __buffer->base = client->_M_alloc_buffer(__suggested_size);
    __buffer->len  = __suggested_size;
}


void
uv_tcp_client::_S_on_data_callback(
    uv_stream_t*    __stream,
    ssize_t         __nread,
    const uv_buf_t* __buffer)
{
    using ready_state = uv_tcp_client::ready_state_t;
    auto* client = reinterpret_cast<uv_tcp_client*>(__stream->data);

    if (__nread > 0) {
        std::string data(__buffer->base, sizeof(char) * __nread);
        client->_M_free_buffer(__buffer->base);
        client->m_bytes_read += data.size();
        client->m_fast_events->on_data(data);
        // reset timeout
        if (client->m_has_timeout)
            client->_M_set_timeout(client->m_timeout);
    }
    else if (__nread == UV_EOF) {
        // clear timeout
        if (client->m_has_timeout)
            client->_M_set_timeout(0);
        if (client->m_ready_state == ready_state::OPEN)
            client->m_ready_state = ready_state::WRITE_ONLY;
        else if (client->m_ready_state == ready_state::READ_ONLY)
            client->_M_on_close();
    }
    else client->_M_emit_error_code("read", __nread);
}


void
uv_tcp_client::_S_on_write_callback(
    uv_write_t* __request,
    int         __status)
{
    auto result = __status;
    auto* context =
        reinterpret_cast<uv_tcp_client::write_context_t*>(__request->data);
    if (result == UV_ECONNRESET)
        context->client->_M_on_close();
    if (result < 0)
        context->client->_M_emit_error_code("write", result);
    else {
        context->client->m_bytes_written += context->data.size();
        if (context->cb)
            context->cb({});
    }
    delete context;
    delete __request;
}


void
uv_tcp_client::_S_on_shutdown_callback(
    uv_shutdown_t* __request,
    int            __status)
{
    auto result = __status;
    auto* client =
        reinterpret_cast<uv_tcp_client*>(__request->data);
    if (result < 0)
        client->_M_emit_error_code("end", result);
    else {
        using ready_state_t = uv_tcp_client::ready_state_t;
        if (client->m_ready_state == ready_state_t::OPEN) {
            client->m_ready_state = ready_state_t::READ_ONLY;
            client->m_fast_events->on_end();
        }
        else client->_M_on_close(); /* WRITE_ONLY state assumed */
    }
    delete __request;
}


void
uv_tcp_client::_S_on_close_callback(uv_handle_t* __handle)
{
    auto* client =
        reinterpret_cast<uv_tcp_client*>(__handle->data);
    client->_M_on_close();
}
