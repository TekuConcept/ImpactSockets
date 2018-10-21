/**
 * Created by TekuConcept on September 24, 2018
 * NOTE: May need to use WinPCap for sending raw frames in Windows
 */

#include "sockets/raw_socket.h"

#include <algorithm>

#if defined(__OS_APPLE__)
    #include <fcntl.h>
    #include <net/bpf.h>
#elif defined(__OS_LINUX__)
    #include <linux/if_packet.h>
    #include <net/ethernet.h>
    #include <cstring>
#elif defined(__OS_WINDOWS__)
    #include <tchar.h>
#endif

#include "utils/impact_error.h"
#include "utils/errno.h"
#include "sockets/generic.h"
#include "basic_socket_common.inc"

#if defined HAVE_NPCAP
    #include <pcap.h>
#endif

using namespace impact;
using namespace experimental;

#include <iostream>
#define VERBOSE(x) std::cout << x << std::endl


std::string
raw_socket::interface_name() const noexcept
{
    return m_interface_name_;
}


/////////////////////////////////////////////////////////////////////
#if defined HAVE_NPCAP
/////////////////////////////////////////////////////////////////////

raw_socket::raw_socket()
{
#if defined __OS_WINDOWS__
    static bool dll_loaded = false;
    if (!dll_loaded) {
        /* Warning: may not be thread-safe! */
        dll_loaded = true;
        /* import npcap DLLs */
        char npcap_dll_directory[512];
        unsigned int length = GetSystemDirectory(npcap_dll_directory, 480);
        if (length == 0) throw impact_error(internal::error_message());
        _tcscat_s(npcap_dll_directory, 512, _T("\\Npcap"));
        if (SetDllDirectory(npcap_dll_directory) == 0)
            throw impact_error(internal::error_message());
    }
#endif
}


raw_socket::~raw_socket() {}


int
raw_socket::send(
    const void* __buffer,
    int         __length)
{
    if (__length <= 0) return 0;
    pcap_t* handle = (pcap_t*)m_pcap_descriptor_.get();

    auto status = pcap_sendpacket(
        handle,
        (unsigned char*)__buffer,
        __length
    );

    if (status == -1)
        throw impact_error(pcap_geterr(handle));

    return __length;
}


int
raw_socket::recv(
    void* __buffer,
    int   __length)
{
    if (__length <= 0) return 0;

    struct pcap_pkthdr* pcap_header;
    const unsigned char* packet_data;
    pcap_t* handle = (pcap_t*)m_pcap_descriptor_.get();

    auto status = pcap_next_ex(
        handle,
        &pcap_header,
        &packet_data);

    if (status == 0) return 0; /* read timeout */
    else if (status == -1)
        throw impact_error(pcap_geterr(handle));

    int size = std::min((unsigned int)__length, pcap_header->caplen);
    memcpy(__buffer, packet_data, size);

    return size;
}


void
raw_socket::attach(std::string __iface_name) {
    try {
#if defined __OS_WINDOWS__
        // NOTE: "\Device\NPF_" prefix is for npcap
        // running in WinPcap compatibility mode.
        std::string prefixed_name = "\\Device\\NPF_" + __iface_name;
#else
        std::string& prefixed_name = __iface_name;
#endif
        m_interface_name_ = prefixed_name;
        _M_attach(prefixed_name.c_str());
    }
    catch (...) { throw; }
}


void
raw_socket::_M_attach(const char* __iface_name)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    const int k_max_packet_caputure_size = 65536;
    const int k_promiscuous_mode         = 1;
    const int k_timeout_ms = 1000;
    pcap_t* handle = pcap_open_live(
        __iface_name,
        k_max_packet_caputure_size,
        k_promiscuous_mode,
        k_timeout_ms,
        errbuf);
    if (handle == NULL)
        throw impact_error(errbuf);

    m_pcap_descriptor_ = std::shared_ptr<void>((void*)handle, [](void* ptr) {
        pcap_close((pcap_t*)ptr);
    });
}

/////////////////////////////////////////////////////////////////////
#elif defined __OS_WINDOWS__ /* dummy interface */
/////////////////////////////////////////////////////////////////////

raw_socket::raw_socket()
{}


raw_socket::~raw_socket()
{}


int
raw_socket::send(
    const void* __buffer,
    int         __length)
{
    UNUSED(__buffer);
    UNUSED(__length);
    return 0;
}


int
raw_socket::recv(
    void* __buffer,
    int   __length)
{
    UNUSED(__buffer);
    UNUSED(__length);
    return 0;
}


void
raw_socket::attach(std::string __iface_name)
{
    m_interface_name_ = __iface_name;
}


void
raw_socket::_M_attach(const char* __iface_name)
{
    UNUSED(__iface_name);
}

/////////////////////////////////////////////////////////////////////
#else /* __OS_LINUX__ | __OS_APPLE__ */
/////////////////////////////////////////////////////////////////////

raw_socket::raw_socket()
{
#if defined(__OS_LINUX__)
    VERBOSE("Raw: Linux Detected");
    m_socket_ = make_socket(
        socket_domain::PACKET,
        socket_type::RAW,
        (socket_protocol)htons(ETH_P_ALL)
    );
#else /* __OS_APPLE__ */
    VERBOSE("Raw: Apple Detected");
    std::string file_name;
    for (auto i = 0; i < 255; i++) {
        file_name = std::string("/dev/bpf") + std::to_string(i);
        m_bpf_descriptor_ = ::open(file_name.c_str(), O_RDWR);
        if (m_bpf_descriptor_ > -1) break;
    }
    if (m_bpf_descriptor_ == -1)
        throw impact_error("Cannot open any /dev/bpf* device");
#endif
}


raw_socket::~raw_socket()
{
#if defined (__OS_APPLE__)
    if (m_bpf_descriptor_ != -1)
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
    if (__length <= 0) return 0;
#if defined(__OS_APPLE__)
    auto status = ::write(
        m_bpf_descriptor_,
        __buffer,
        __length
    );
#else /* __OS_LINUX__ */
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
    if (__length <= 0) return 0;
#if defined(__OS_APPLE__)
    auto status = ::read(
        m_bpf_descriptor_,
        &m_aligned_buffer_[0],
        m_aligned_buffer_.size()
    );
    auto berkley_packet_header = (struct bpf_hdr*)&m_aligned_buffer_[0];
    auto size = std::min((unsigned int)__length,
        berkley_packet_header->bh_caplen);
    memcpy(__buffer, &m_aligned_buffer_[0] + berkley_packet_header->bh_hdrlen,
        size);
    status = size;
    ASSERT(status != SOCKET_ERROR)
#else /* __OS_LINUX__ */
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
raw_socket::attach(std::string __iface_name)
{
    m_interface_name_ = __iface_name;
    try { _M_attach(__iface_name.c_str()); }
    catch (...) { throw; }
}


void
raw_socket::_M_attach(const char* __interface_name)
{
    struct ifreq ifr;
    strncpy(
        ifr.ifr_name,
        __interface_name,
        sizeof(ifr.ifr_name) - 1);

#if defined(__OS_APPLE__)
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
    unsigned int buffer_align_size = 0;
    status = ::ioctl(m_bpf_descriptor_, BIOCGBLEN, &buffer_align_size);
    m_aligned_buffer_.resize(buffer_align_size);
    if (status < 0) throw impact_error(
        std::string("ioctl error: ") +
        std::to_string(status));
    VERBOSE("Raw: [Apple] buffer alignment " << m_aligned_buffer_.size());

#elif defined(__LINUX__)
    auto status = ::ioctl(m_socket_.get(), SIOCGIFINDEX, &ifr);
    if (status < 0) throw impact_error(
        std::string("ioctl error: ") +
        std::to_string(status));
    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(sll));
    sll.sll_family = PF_PACKET;
    sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_ifindex = ifr.ifr_ifindex;
    status = ::bind(m_socket_.get(), (struct sockaddr*)&sll, sizeof(sll));
    ASSERT(status != SOCKET_ERROR)
#endif
}

#endif
