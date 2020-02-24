/**
 * Created by TekuConcept on September 24, 2018
 * WARNING: Experimental across platforms
 */

#ifndef IMPACT_RAW_SOCKET_H
#define IMPACT_RAW_SOCKET_H

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

        void attach(std::string interface_name)
            /* throw(impact_error) */;

        std::string interface_name() const noexcept;

    private:
        std::string m_interface_name_;

    #if defined HAVE_NPCAP
        // using void* in header to hide library-specific dependencies
        std::shared_ptr<void> m_pcap_descriptor_;
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
        std::vector<unsigned char> m_aligned_buffer_;
        #endif
    #endif
    
        void _M_attach(const char*);
    };
}}

#endif
