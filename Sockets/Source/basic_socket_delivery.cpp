/**
 * Created by TekuConcept on July 11, 2018
 */

#include "sockets/basic_socket.h"
#include "basic_socket_common.inc"

using namespace impact;

void
basic_socket::bind(unsigned short __port)
{
    ASSERT_MOVED
    struct sockaddr_in socket_address;

    ::memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family      = AF_INET;
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_address.sin_port        = htons(__port);

    auto status = ::bind(
        m_info_->descriptor,
        (struct sockaddr*)&socket_address,
        sizeof(socket_address)
    );

    ASSERT(status != SOCKET_ERROR)
}


void
basic_socket::bind(
    std::string    __address,
    unsigned short __port)
{
    ASSERT_MOVED
    std::shared_ptr<struct sockaddr> socket_address;

    size_t size;
    CATCH_ASSERT(
        size = internal::fill_address(
            m_info_->domain,
            m_info_->type,
            m_info_->protocol,
            __address,
            __port,
            &socket_address
        );
    )

    auto status = ::bind(
        m_info_->descriptor,
        socket_address.get(),
        size
    );

    ASSERT(status != SOCKET_ERROR)
}


void
basic_socket::bind(const struct sockaddr& __address)
{
    ASSERT_MOVED

    auto status = ::bind(
        m_info_->descriptor,
        &__address,
        sizeof(__address)
    );

    ASSERT(status != SOCKET_ERROR)
}

#include <iostream>
#include <iomanip>
#include "sockets/networking.h"
#define VERBOSE(x) std::cout << x << std::endl
void
basic_socket::connect(
    unsigned short __port,
    std::string    __address)
{
    ASSERT_MOVED
    std::shared_ptr<struct sockaddr> destination_address;

    size_t size;
    CATCH_ASSERT(
        size = internal::fill_address(
            m_info_->domain,
            m_info_->type,
            m_info_->protocol,
            __address,
            __port,
            &destination_address
        );
    )

    auto status = ::connect(
        m_info_->descriptor,
        destination_address.get(),
        size
    );

    ASSERT(status != SOCKET_ERROR)
}


void
basic_socket::listen(int __backlog)
{
    ASSERT_MOVED
    auto status = ::listen(m_info_->descriptor, __backlog);
    ASSERT(status != SOCKET_ERROR)
}


basic_socket
basic_socket::accept()
{
    ASSERT_MOVED
    basic_socket peer;
    peer.m_info_->descriptor = ::accept(m_info_->descriptor, NULL, NULL);
    ASSERT(peer.m_info_->descriptor != INVALID_SOCKET)
    peer.m_info_->wsa        = false;
    peer.m_info_->domain     = m_info_->domain;
    peer.m_info_->type       = m_info_->type;
    peer.m_info_->protocol   = m_info_->protocol;
    return peer;
}


void
basic_socket::shutdown(socket_channel __channel)
{
    ASSERT_MOVED
    auto status = ::shutdown(m_info_->descriptor, (int)__channel);
    ASSERT(status != SOCKET_ERROR)
}


void
basic_socket::group(
    std::string            __name,
    group_application      __method)
{
    ASSERT_MOVED
    struct ip_mreq multicast_request;
    multicast_request.imr_multiaddr.s_addr = inet_addr(__name.c_str());
    multicast_request.imr_interface.s_addr = htonl(INADDR_ANY);
    auto status = ::setsockopt(
        m_info_->descriptor,
        IPPROTO_IP,
        (int)__method,
        (CCHAR_PTR)&multicast_request,
        sizeof(multicast_request)
    );
    ASSERT(status != SOCKET_ERROR)
}


void
basic_socket::keepalive(struct keep_alive_options __options)
{
    ASSERT_MOVED
    // http://helpdoco.com/C++-C/how-to-use-tcp-keepalive.htm
    std::ostringstream os;
    auto errors = 0;
    auto status = 0;
#if defined(__OS_WINDOWS__)
    DWORD bytes_returned     = 0;
    struct tcp_keepalive config;
    config.onoff             = __options.enabled;
    config.keepalivetime     = __options.idletime;
    config.keepaliveinterval = __options.interval;

    status = WSAIoctl(
        m_info_->descriptor,
        SIO_KEEPALIVE_VALS,
        &config,
        sizeof(config),
        NULL,
        0,
        &bytes_returned,
        NULL,
        NULL
    );

    ASSERT(status != SOCKET_ERROR)
#else /* OSX|LINUX */
    status = setsockopt(
        m_info_->descriptor,
        SOL_SOCKET,
        SO_KEEPALIVE,
        (const char*)&__options.enabled,
        sizeof(int)
    );

    if (status == SOCKET_ERROR) {
        os << "[keepalive] ";
        os << internal::error_message();
        os << std::endl;
        errors |= 1;
    }
#ifndef __OS_APPLE__
    status = setsockopt(
        m_info_->descriptor,
        IPPROTO_TCP,
        TCP_KEEPIDLE,
        (const char*)&__options.idletime,
        sizeof(int)
    );

    if (status == SOCKET_ERROR) {
        os << "[idle] ";
        os << internal::error_message();
        os << std::endl;
        errors |= 8;
    }
#endif /* __APPLE__ */
    status = setsockopt(
        m_info_->descriptor,
        IPPROTO_TCP,
        TCP_KEEPINTVL,
        (const char*)&__options.interval,
        sizeof(int)
    );

    if (status == SOCKET_ERROR) {
        os << "[interval] ";
        os << internal::error_message();
        os << std::endl;
        errors |= 2;
    }
#endif /* UNIX|LINUX */
    status = setsockopt(
        m_info_->descriptor,
        IPPROTO_TCP,
        TCP_KEEPCNT,
        (const char*)&__options.retries,
        sizeof(int)
    );

    if (status == SOCKET_ERROR) {
        os << "[count] ";
        os << internal::error_message();
        os << std::endl;
        errors |= 4;
    }

    if (errors) throw impact_error(os.str());
}


int
basic_socket::send(
    const void*        __buffer,
    int                __length,
    message_flags      __flags)
{
    ASSERT_MOVED
    auto status = ::send(
        m_info_->descriptor,
        (CCHAR_PTR)__buffer,
        __length,
        (int)__flags
    );
    ASSERT(status != SOCKET_ERROR)
    return status;
}


int
basic_socket::sendto(
    const void*        __buffer,
    int                __length,
    unsigned short     __port,
    const std::string& __address,
    message_flags      __flags)
{
    ASSERT_MOVED
    std::shared_ptr<struct sockaddr> destination_address;

    size_t size;
    CATCH_ASSERT(
        size = internal::fill_address(
            m_info_->domain,
            m_info_->type,
            m_info_->protocol,
            __address,
            __port,
            &destination_address
        );
    )

    auto status = ::sendto(
        m_info_->descriptor,
        (CCHAR_PTR)__buffer,
        __length,
        (int)__flags,
        destination_address.get(),
        size
    );

    ASSERT(status != SOCKET_ERROR)
    return status;
}


int
basic_socket::recv(
    void*              __buffer,
    int                __length,
    message_flags      __flags)
{
    ASSERT_MOVED
    int status = ::recv(
        m_info_->descriptor,
        (CHAR_PTR)__buffer,
        __length,
        (int)__flags
    );
    ASSERT(status != SOCKET_ERROR)
    return status; /* number of bytes received or EOF */
}


int
basic_socket::recvfrom(
    void*              __buffer,
    int                __length,
    unsigned short*    __port,
    std::string*       __address,
    message_flags      __flags)
{
    ASSERT_MOVED
    struct sockaddr_in client_address;
    socklen_t address_length = sizeof(client_address);

    auto status = ::recvfrom(
        m_info_->descriptor,
        (CHAR_PTR)__buffer,
        __length,
        (int)__flags,
        (struct sockaddr*)&client_address,
        (socklen_t*)&address_length
    );

    ASSERT(status != SOCKET_ERROR)

    if (__address)
        *__address = inet_ntoa(client_address.sin_addr);
    if (__port)
        *__port    = ntohs(client_address.sin_port);

    return status;
}
