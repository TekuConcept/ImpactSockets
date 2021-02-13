/**
 * Created by TekuConcept on July 11, 2018
 */

#include "sockets/basic_socket.h"
#include "basic_socket_common.h"

using namespace impact;

std::string
basic_socket::local_address()
{
    IMPACT_ASSERT_MOVED
    struct sockaddr_storage address;
    auto address_length = sizeof(address);

    auto status = ::getsockname(
        m_info_->descriptor,
        (struct sockaddr*)&address,
        (socklen_t*)&address_length
    );
    IMPACT_ASSERT(status != SOCKET_ERROR)

    void* source;
    std::string result;
    switch ((address_family)address.ss_family) {
    case address_family::INET:
        source = &((sockaddr_in* )&address)->sin_addr;
        result.resize(INET_ADDRSTRLEN);
        break;
    case address_family::INET6:
        source = &((sockaddr_in6*)&address)->sin6_addr;
        result.resize(INET6_ADDRSTRLEN);
        break;
    default: throw impact_error("feature not supported"); break;
    }

    auto status2 = inet_ntop(
        address.ss_family, source, &result[0], result.size());
    if (status2 == nullptr)
        throw impact_error("unexpected error");

    return result;
}


unsigned short
basic_socket::local_port()
{
    IMPACT_ASSERT_MOVED
    struct sockaddr_storage address;
    auto address_length = sizeof(address);

    auto status = ::getsockname(
        m_info_->descriptor,
        (struct sockaddr*)&address,
        (socklen_t*)&address_length
    );
    IMPACT_ASSERT(status != SOCKET_ERROR)

    unsigned short port;
    switch ((address_family)address.ss_family) {
    case address_family::INET:  port = ((sockaddr_in* )&address)->sin_port;  break;
    case address_family::INET6: port = ((sockaddr_in6*)&address)->sin6_port; break;
    default: throw impact_error("feature not supported"); break;
    }

    return ntohs(port);
}


std::string
basic_socket::peer_address()
{
    IMPACT_ASSERT_MOVED
    struct sockaddr_storage address;
    unsigned int address_length = sizeof(address);

    auto status = ::getpeername(
        m_info_->descriptor,
        (struct sockaddr*)&address,
        (socklen_t*)&address_length
    );
    IMPACT_ASSERT(status != SOCKET_ERROR)

    void* source;
    std::string result;
    switch ((address_family)address.ss_family) {
    case address_family::INET:
        source = &((sockaddr_in* )&address)->sin_addr;
        result.resize(INET_ADDRSTRLEN);
        break;
    case address_family::INET6:
        source = &((sockaddr_in6*)&address)->sin6_addr;
        result.resize(INET6_ADDRSTRLEN);
        break;
    default: throw impact_error("feature not supported"); break;
    }

    auto status2 = inet_ntop(
        address.ss_family, source, &result[0], result.size());
    if (status2 == nullptr)
        throw impact_error("unexpected error");

    return result;
}


unsigned short
basic_socket::peer_port()
{
    IMPACT_ASSERT_MOVED
    struct sockaddr_storage address;
    unsigned int address_length = sizeof(address);

    auto status = ::getpeername(
        m_info_->descriptor,
        (struct sockaddr*)&address,
        (socklen_t*)&address_length
    );
    IMPACT_ASSERT(status != SOCKET_ERROR)

    unsigned short port;
    switch ((address_family)address.ss_family) {
    case address_family::INET:  port = ((sockaddr_in* )&address)->sin_port;  break;
    case address_family::INET6: port = ((sockaddr_in6*)&address)->sin6_port; break;
    default: throw impact_error("feature not supported"); break;
    }

    return ntohs(port);
}


void
basic_socket::broadcast(bool __enabled)
{
    IMPACT_ASSERT_MOVED
    auto permission = __enabled ? 1 : 0;

    auto status = ::setsockopt(
        m_info_->descriptor,
        SOL_SOCKET,
        SO_BROADCAST,
        (CCHAR_PTR)&permission,
        sizeof(permission)
    );

    IMPACT_ASSERT(status != SOCKET_ERROR)
}


void
basic_socket::multicast_ttl(unsigned char __ttl)
{
    IMPACT_ASSERT_MOVED

    auto status = ::setsockopt(
        m_info_->descriptor,
        IPPROTO_IP,
        IP_MULTICAST_TTL,
        (CCHAR_PTR)&__ttl,
        sizeof(__ttl)
    );

    IMPACT_ASSERT(status != SOCKET_ERROR)
}


void
basic_socket::reuse_address(bool __enabled)
{
    IMPACT_ASSERT_MOVED
    int reuse = __enabled ? 1 : 0;
    auto option = SO_REUSEADDR;
#ifdef IMPACT_WIN_SECURE_REUSE
    option = SO_EXCLUSIVEADDRUSE;
#endif
    auto status = ::setsockopt(
        m_info_->descriptor,
        SOL_SOCKET,
        option,
        (const char*)&reuse,
        sizeof(reuse)
    );
    IMPACT_ASSERT(status != SOCKET_ERROR)
#ifdef SO_REUSEPORT
    status = ::setsockopt(
        m_info_->descriptor,
        SOL_SOCKET,
        SO_REUSEPORT,
        (const char*)&reuse,
        sizeof(reuse)
    );
    IMPACT_ASSERT(status != SOCKET_ERROR)
#endif
}
