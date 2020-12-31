/**
 * Created by TekuConcept on July 11, 2018
 */

#include "sockets/basic_socket.h"
#include "basic_socket_common.h"

using namespace impact;


unsigned short
basic_socket::_M_resolve_service(
    const std::string& __service,
    const std::string& __protocol)
{
    struct servent* service_info = ::getservbyname(__service.c_str(),
        __protocol.c_str());
    if (service_info == NULL)
        return static_cast<unsigned short>(atoi(__service.c_str()));
    /* Type of service is the port number ie 80/http*/
    else return ntohs(service_info->s_port);
    /* Found port (network byte order) by name */
}


void
basic_socket::_M_copy(const basic_socket& __rvalue)
{
    if (__rvalue.m_info_ == nullptr)
        throw impact_error("Copying moved socket");
    m_info_ = __rvalue.m_info_;
}


void
basic_socket::_M_move(basic_socket&& __rvalue)
{
    if (__rvalue.m_info_ == nullptr)
        throw impact_error("Moving already moved object");
    m_info_          = __rvalue.m_info_;
    __rvalue.m_info_ = nullptr;
}


void
basic_socket::_M_dtor()
{
    if (m_info_ != nullptr && m_info_.use_count() == 1) {
        IMPACT_TRY_BEGIN
        if (m_info_->descriptor != INVALID_SOCKET)
            close();
        IMPACT_TRY_END
        m_info_->descriptor = INVALID_SOCKET;
        m_info_ = nullptr;
    }
    // else moved
}


basic_socket::basic_socket()
{
#if !defined(__OS_WINDOWS__)
    try { internal::no_sigpipe(); } catch (...) { /* do nothing */ }
#endif
    m_info_              = std::make_shared<basic_socket_info>();
    m_info_->wsa         = false;
    m_info_->descriptor  = INVALID_SOCKET;
    m_info_->domain      = address_family::UNSPECIFIED;
    m_info_->type        = socket_type::RAW;
    m_info_->protocol    = internet_protocol::DEFAULT;
}


basic_socket::basic_socket(
    address_family    __domain,
    socket_type       __type,
    internet_protocol __proto)
{
    #if defined(__OS_WINDOWS__)
        static WSADATA wsa_data;
        auto status = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        WIN_ASSERT(status == 0, status, (void)0;)
        result.m_info_->wsa    = true;
    #endif
    m_info_->descriptor = ::socket((int)__domain, (int)__type, (int)__proto);
    IMPACT_ASSERT(m_info_->descriptor != INVALID_SOCKET);
    m_info_->domain     = __domain;
    m_info_->type       = __type;
    m_info_->protocol   = __proto;
}


basic_socket::basic_socket(const basic_socket& __rvalue)
{
    IMPACT_TRY_BEGIN
    _M_copy(__rvalue);
    IMPACT_TRY_END
}


basic_socket::basic_socket(basic_socket&& __rvalue)
{
    IMPACT_TRY_BEGIN
    _M_move(std::move(__rvalue));
    IMPACT_TRY_END
}


basic_socket
impact::make_socket(
    address_family    __domain,
    socket_type       __type,
    internet_protocol __proto)
{
    return basic_socket(__domain, __type, __proto);
}


basic_socket
impact::make_tcp_socket()
{
    IMPACT_TRY_BEGIN
        return basic_socket(
            address_family::INET,
            socket_type::STREAM,
            internet_protocol::TCP
        );
    IMPACT_TRY_END
}


basic_socket
impact::make_udp_socket()
{
    IMPACT_TRY_BEGIN
        return basic_socket(
            address_family::INET,
            socket_type::DATAGRAM,
            internet_protocol::UDP
        );
    IMPACT_TRY_END
}


basic_socket::~basic_socket()
{
    try { _M_dtor(); }
    catch (...) { /* do nothing */ }
}


void
basic_socket::close()
{
    IMPACT_ASSERT_MOVED
    auto status = CLOSE_SOCKET(m_info_->descriptor);
    m_info_->descriptor = INVALID_SOCKET;
    IMPACT_ASSERT(status != SOCKET_ERROR)
#if defined(__WINDOWS__)
    if (m_info_->wsa)
        WSACleanup();
    m_info_->wsa = false;
#endif
}


basic_socket&
basic_socket::operator=(const basic_socket& __rvalue)
{
    IMPACT_TRY_BEGIN
    if (m_info_ && m_info_.use_count() > 0)
        _M_dtor();
    _M_copy(__rvalue);
    IMPACT_TRY_END
    return *this;
}


basic_socket&
basic_socket::operator=(basic_socket&& __rvalue)
{
    IMPACT_TRY_BEGIN
    if (m_info_ && m_info_.use_count() > 0)
        _M_dtor();
    _M_move(std::move(__rvalue));
    IMPACT_TRY_END
    return *this;
}


long
basic_socket::use_count() const noexcept
{
    return m_info_.use_count();
}


int
basic_socket::get() const noexcept
{
    if (m_info_) return m_info_->descriptor;
    else return INVALID_SOCKET;
}


address_family
basic_socket::domain() const noexcept
{
    if (m_info_) return m_info_->domain;
    else return address_family::UNSPECIFIED;
}


socket_type
basic_socket::type() const noexcept
{
    if (m_info_) return m_info_->type;
    else return socket_type::RAW;
}


internet_protocol
basic_socket::protocol() const noexcept
{
    if (m_info_) return m_info_->protocol;
    else return internet_protocol::DEFAULT;
}


basic_socket::operator bool() const noexcept
{
    if (m_info_)
        return m_info_->descriptor != INVALID_SOCKET;
    else return false;
}
