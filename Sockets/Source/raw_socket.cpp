/**
 * Created by TekuConcept on September 24, 2018
 * NOTE: May need to use WinPCap for sending raw frames
 */

#include "sockets/raw_socket.h"

#include <fcntl.h>

#include "utils/environment.h"
#include "utils/impact_error.h"
#include "basic_socket_common.inc"

#if defined(__APPLE__)
    #include <net/bpf.h>
#endif

using namespace impact;
using namespace experimental;


raw_socket::raw_socket()
{
#if defined(__LINUX__)
    m_socket_ = make_socket(
        socket_domain::PACKET,
        socket_type::RAW,
        htons(ETH_P_ALL)
    );
#elif defined(__WINDOWS__)
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
    std::string file_name;
    for (auto i = 0; i < 99; i++) {
        file_name = std::string("/dev/bpf") + std::to_string(i);
        m_bpf_descriptor_ = ::open( file_name.c_str(), O_RDWR );
        if( m_bpf_descriptor_ != -1 ) break;
    }
    if (m_bpf_descriptor_ == -1)
        throw impact_error("Cannot open any /dev/bpf* device");
#endif
}


raw_socket::~raw_socket()
{
#if defined (__APPLE__)
    ::close(m_bpf_descriptor_);
#else
    try { m_socket_.close(); }
    catch (...) { /* silently ignore errors */ }
#endif
}


int
raw_socket::send(
    const void* __buffer,
    int         __length)
{
    #if defined(__APPLE__)
        auto target = m_bpf_descriptor_;
    #else
        auto target = m_socket_.get();
    #endif
    auto status = ::send(
		target,
		(CCHAR_PTR)__buffer,
		__length,
		(int)message_flags::NONE
	);
	ASSERT(status != SOCKET_ERROR)
	return status;
}


int
raw_socket::recv(
    void* __buffer,
    int   __length)
{
    #if defined(__APPLE__)
        auto target = m_bpf_descriptor_;
    #else
        auto target = m_socket_.get();
    #endif
    int status = ::recv(
		target,
		(CHAR_PTR)__buffer,
		__length,
		(int)message_flags::NONE
	);
	ASSERT(status != SOCKET_ERROR)
	return status;
}


void
raw_socket::associate(std::string __interface_name)
{
    auto interface_list = networking::find_network_interfaces();
    for (const auto& iface : interface_list) {
        if (iface.name == __interface_name) {
            m_interface_ = iface;
            break;
        }
    }

#if defined(__WINDOWS__)
    m_socket_.bind(m_interface_.address, 0);
#else
    #if defined(__APPLE__)
        auto target = m_bpf_descriptor_;
        auto flag   = BIOCSETIF;
    #elif
        auto target = m_socket_.get();
        auto flag   = SIOCGIFINDEX;
    #endif
        struct ifreq ifr;
        strcpy(ifr.ifr_name, __interface_name.c_str());
        auto status  = ::ioctl(target, flag, &ifr);
        if (status > 0)
            throw impact_error(
                std::string("ioctl error: ") +
                std::to_string(status));
    #if defined(__LINUX__)
        struct sockaddr_ll sll;
        memset(&sll, 0, sizeof(sll));
        sll.sll_family   = PF_PACKET;
        sll.sll_protocol = htons(ETH_P_ALL);
        sll.sll_ifindex  = ifr.ifr_ifindex;
        status = ::bind(target, (struct sockaddr*)&sll, sizeof(sll));
        ASSERT(status != SOCKET_ERROR)
    #endif
#endif
}


const struct networking::netinterface&
raw_socket::interface() const noexcept
{
    return m_interface_;
}
