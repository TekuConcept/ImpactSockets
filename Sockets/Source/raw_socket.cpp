/**
 * Created by TekuConcept on September 24, 2018
 * NOTE: May need to use WinPCap for sending raw frames in Windows
 */

#include "sockets/raw_socket.h"

#if defined(__OS_APPLE__)
    #include <fcntl.h>
    #include <net/bpf.h>
    #include <algorithm> /* std::min */
#elif defined(__OS_LINUX__)
    #include <linux/if_packet.h>
    #include <net/ethernet.h>
    #include <string.h>
#endif

#include "utils/impact_error.h"
#include "basic_socket_common.inc"

using namespace impact;
using namespace experimental;

#include <iostream>
#define VERBOSE(x)


raw_socket::raw_socket()
: m_buffer_align_size_(4096) /* 4096 is from OSX-BPF */
{
#if defined(__OS_LINUX__)
    VERBOSE("Raw: Linux Detected");
    m_socket_ = make_socket(
        socket_domain::PACKET,
        socket_type::RAW,
        (socket_protocol)htons(ETH_P_ALL)
    );
#elif defined(__OS_WINDOWS__)
    VERBOSE("Raw: Windows Detected");
    m_socket_ = make_socket(
        socket_domain::INET,
        socket_type::RAW,
        socket_protocol::RAW
    );
    // header included as part of raw data
    int optval = 1;
    auto status = ::setsockopt(
        m_socket_.get(),
        IPPROTO_IP,
        IP_HDRINCL,
        (char*)&optval,
        sizeof(optval)
    );
    ASSERT(status != SOCKET_ERROR)
#else
    VERBOSE("Raw: Apple Detected");
    std::string file_name;
    for (auto i = 0; i < 255; i++) {
        file_name = std::string("/dev/bpf") + std::to_string(i);
        m_bpf_descriptor_ = ::open( file_name.c_str(), O_RDWR );
        if( m_bpf_descriptor_ > -1 ) break;
    }
    if (m_bpf_descriptor_ == -1)
        throw impact_error("Cannot open any /dev/bpf* device");
#endif
}


raw_socket::~raw_socket()
{
#if defined (__OS_APPLE__)
    VERBOSE("Raw: [Apple] dtor");
    if (m_bpf_descriptor_ != -1)
        ::close(m_bpf_descriptor_);
#else
    VERBOSE("Raw: [Other] dtor");
    try { m_socket_.close(); }
    catch (...) { /* silently ignore errors */ }
#endif
}


int
raw_socket::send(
    const void* __buffer,
    int         __length)
{
#if defined(__OS_APPLE__)
    VERBOSE("Raw: [Apple] send");
    auto status = ::write(
        m_bpf_descriptor_,
        __buffer,
        __length
    );
#else
    VERBOSE("Raw: [Other] send");
    auto status = ::send(
        m_socket_.get(),
        (CCHAR_PTR)__buffer,
        __length,
        (int)message_flags::NONE
    );
#endif
    ASSERT(status != SOCKET_ERROR)
    return status;
}


int
raw_socket::recv(
    void* __buffer,
    int   __length)
{
    if (__length <= 0) throw impact_error("invalid length");
#if defined(__OS_APPLE__)
    VERBOSE(
        "Raw: [Apple] recv [" <<
        m_bpf_descriptor_ << ", " <<
        __buffer << ", " <<
        m_buffer_align_size_ << "]");
    auto status = ::read(
        m_bpf_descriptor_,
        __buffer,
        __length
    );
    ASSERT(status != SOCKET_ERROR)
#else
    VERBOSE("Raw: [Other] recv");
    auto status = ::recv(
        m_socket_.get(),
        (CHAR_PTR)__buffer,
        __length,
        (int)message_flags::NONE
    );
    ASSERT(status != SOCKET_ERROR)
#endif
    return status;
}


void
raw_socket::associate(std::string __interface_name)
{
    VERBOSE("Raw: associating...");
    auto interface_list = networking::find_network_interfaces();
    for (const auto& iface : interface_list) {
        if (iface.name == __interface_name) {
            m_interface_ = iface;
            VERBOSE("Raw: Found iface " << iface.name);
            if (m_interface_.ipv4) break; // prefer ipv4
        }
    }
    if (m_interface_.name != __interface_name)
        throw impact_error(
            std::string("No iface name \"") +
            __interface_name +
            std::string("\" detected"));

    try { _M_associate(__interface_name.c_str()); } catch (...) { throw; }
}


void
raw_socket::associate(struct networking::netinterface __iface)
{
    m_interface_ = __iface;
    try { _M_associate(__iface.name.c_str()); } catch (...) { throw; }
}


void
raw_socket::_M_associate(const char* __interface_name)
{
#if defined(__OS_WINDOWS__)
    VERBOSE("Raw: [Windows] bind");
    m_socket_.bind(m_interface_.address.get(), 0);
#else
    struct ifreq ifr;
    strncpy(
        ifr.ifr_name,
        __interface_name,
        sizeof(ifr.ifr_name) - 1);

    #if defined(__OS_APPLE__)
        VERBOSE("Raw: [Apple] associated");
        unsigned int enabled = 1;
        auto target = m_bpf_descriptor_;
        auto status = ::ioctl(target, BIOCSETIF, &ifr);
        if (status < 0) throw impact_error(
            std::string("ioctl error: ") +
            std::to_string(status));
        status = ::ioctl(target, BIOCSHDRCMPLT, &enabled);
        if (status < 0) throw impact_error(
            std::string("ioctl error: ") +
            std::to_string(status));
        status = ::ioctl(target, BIOCIMMEDIATE, &enabled);
        if (status < 0) throw impact_error(
            std::string("ioctl error: ") +
            std::to_string(status));
            m_buffer_align_size_ = 0; // clear to be rewritten
        status = ::ioctl(m_bpf_descriptor_, BIOCGBLEN, &m_buffer_align_size_);
        if (status < 0) throw impact_error(
            std::string("ioctl error: ") +
            std::to_string(status));
        VERBOSE("Raw: [Apple] buffer alignment " << m_buffer_align_size_);

    #elif defined(__LINUX__)
        VERBOSE("Raw: [Linux] associated");
        auto status = ::ioctl(m_socket_.get(), SIOCGIFINDEX, &ifr);
        if (status < 0) throw impact_error(
            std::string("ioctl error: ") +
            std::to_string(status));
        struct sockaddr_ll sll;
        memset(&sll, 0, sizeof(sll));
        sll.sll_family   = PF_PACKET;
        sll.sll_protocol = htons(ETH_P_ALL);
        sll.sll_ifindex  = ifr.ifr_ifindex;
        status = ::bind(m_socket_.get(), (struct sockaddr*)&sll, sizeof(sll));
        ASSERT(status != SOCKET_ERROR)
    #endif
#endif
}


const struct networking::netinterface&
raw_socket::iface() const noexcept
{
    return m_interface_;
}


int
raw_socket::get() const noexcept
{
#if defined(__OS_APPLE__)
    return m_bpf_descriptor_;
#else
    return m_socket_.get();
#endif
}


int
raw_socket::allignment() const noexcept
{
    return m_buffer_align_size_;
}
