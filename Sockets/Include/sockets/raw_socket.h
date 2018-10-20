/**
 * Created by TekuConcept on September 24, 2018
 * WARNING: Experimental across platforms
 */

#ifndef _IMPACT_RAW_SOCKET_H_
#define _IMPACT_RAW_SOCKET_H_

#include <memory>
#include <string>

#include "utils/environment.h"
#include "sockets/networking.h"
#include "sockets/basic_socket.h"

namespace impact {
namespace experimental {
    class raw_socket {
    public:
        raw_socket()
            /* throw(impact_error) */;
        virtual ~raw_socket();

        int send(const void* buffer, int length)
            /* throw(impact_error) */;
        int recv(void* buffer, int length)
            /* throw(impact_error) */;

        void associate(struct networking::netinterface iface)
            /* throw(impact_error) */;

        const struct networking::netinterface& iface() const noexcept;
        int allignment() const noexcept;

        std::shared_ptr<void> m_pcap_descriptor_;

    private:
        struct networking::netinterface m_interface_;
        size_t m_buffer_align_size_;

    #if defined HAVE_NPCAP
        // using void* in header to hide library-specific dependencies
        bool _M_iterative_find(const struct networking::netinterface&,
            const void*,std::string*);
    #else
        #if defined __OS_LINUX__
        basic_socket m_socket_;
        #elif defined __OS_WINDOWS__
        // Windows natively doesn't support raw sockets for
        // security reasons. Without a packet filter driver
        // raw_socket can only act as a dummy class
        #else
        // OSX and BSD systems use BPF devices for raw networking 
        int m_bpf_descriptor_;
        #endif
    #endif
    
        void _M_associate(const char*);
    };
}}

#endif
