/**
 * Created by TekuConcept on September 24, 2018
 * WARNING: Experimental across platforms
 */

#ifndef _IMPACT_RAW_SOCKET_H_
#define _IMPACT_RAW_SOCKET_H_

#include <memory>
#include <string>

#include "sockets/basic_socket.h"
#include "sockets/networking.h"

namespace impact {
namespace experimental {
    class raw_socket {
    public:
        raw_socket();
        virtual ~raw_socket();
        
        int send(const void* buffer, int length)
            /* throw(impact_error) */;
        int recv(void* buffer, int length)
            /* throw(impact_error) */;
        // sendto
        // recvfrom
        
        void associate(std::string interface_name)
            /* throw(impact_error) */;
        
        const struct networking::netinterface& interface() const noexcept;

    private:
        struct networking::netinterface m_interface_;
    #if defined(__LINUX__) || defined(__WINDOWS__)
        basic_socket m_socket_;
    #else
        // OSX and BSD systems use BPF devices for raw networking 
        int m_bpf_descriptor_;
    #endif
    };
}}

#endif