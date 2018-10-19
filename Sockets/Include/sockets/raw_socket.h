/**
 * Created by TekuConcept on September 24, 2018
 * WARNING: Experimental across platforms
 */

#ifndef _IMPACT_RAW_SOCKET_H_
#define _IMPACT_RAW_SOCKET_H_

#include <memory>
#include <string>

#include "utils/environment.h"
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

        void associate(std::string interface_name)
            /* throw(impact_error) */;
        void associate(struct networking::netinterface iface)
            /* throw(impact_error) */;

        const struct networking::netinterface& iface() const noexcept;
        int get() const noexcept;
        int allignment() const noexcept;

    private:
        struct networking::netinterface m_interface_;
        size_t m_buffer_align_size_;
    #if defined(__OS_LINUX__) || defined(__OS_WINDOWS__)
        basic_socket m_socket_;
    #else
        // OSX and BSD systems use BPF devices for raw networking 
        int m_bpf_descriptor_;
    #endif
    
        void _M_associate(const char*);
    };
}}

#endif
