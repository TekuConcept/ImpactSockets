/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef _IMPACT_BASIC_SOCKET_H_
#define _IMPACT_BASIC_SOCKET_H_

#include <atomic>
#include <string>
#include <vector>
#include <future>
#include <memory>

#include "sockets/types.h"

namespace impact {
    class basic_socket {
    public:
        enum {
            INVALID = -1
        };

        // constructors
        basic_socket();
        basic_socket(const basic_socket& r) /* throw(impact_error) */;
        basic_socket(basic_socket&& r) /* throw(impact_error) */;

        // destructors
        virtual ~basic_socket();

        // file operators
        void close() /* throw(impact_error) */;

        // assignment
        basic_socket& operator=(const basic_socket& r)
            /* throw(impact_error) */;
        basic_socket& operator=(basic_socket&& r)
            /* throw(impact_error) */;

        // observers
        long use_count()           const noexcept;
        int get()                  const noexcept;
        socket_domain domain()     const noexcept;
        socket_type type()         const noexcept;
        socket_protocol protocol() const noexcept;
        explicit operator bool()   const noexcept;

        // communication / delivery
        void bind(unsigned short port) /* throw(impact_error) */;
        void bind(std::string address, unsigned short port = 0)
            /* throw(impact_error) */;
        void bind(const sockaddr* address, unsigned short port)
            /* throw(impact_error) */;
        void connect(unsigned short port,
            std::string address = "localhost")
            /* throw(impact_error) */;
        void listen(int backlog = 5)
            /* throw(impact_error) */;
        basic_socket accept()
            /* throw(impact_error) */;
        void shutdown(socket_channel channel = socket_channel::BOTH)
            /* throw(impact_error) */;
        void group(std::string multicast_name, group_application method)
            /* throw(impact_error) */;
        void keepalive(struct keep_alive_options options)
            /* throw(impact_error) */;
        int send(const void* buffer, int length,
            message_flags flags = message_flags::NONE)
            /* throw(impact_error) */;
        int sendto(const void* buffer, int length, unsigned short port,
            const std::string& address,
            message_flags flags = message_flags::NONE)
            /* throw(impact_error) */;
        int recv(void* buffer, int length,
            message_flags flags = message_flags::NONE)
            /* throw(impact_error) */;
        int recvfrom(void* buffer, int length, unsigned short* port,
            std::string* address, message_flags flags = message_flags::NONE)
            /* throw(impact_error) */;

        // miscillaneous
        std::string local_address()  /* throw(impact_error) */;
        unsigned short local_port()  /* throw(impact_error) */;
        std::string peer_address()   /* throw(impact_error) */;
        unsigned short peer_port()   /* throw(impact_error) */;
        void broadcast(bool enabled) /* throw(impact_error) */;
        void multicast_ttl(unsigned char time_to_live = 1)
            /* throw(impact_error) */;
        void reuse_address(bool enabled)
            /* throw(impact_error) */;

        friend basic_socket make_socket(
            socket_domain, socket_type, socket_protocol);
        friend basic_socket make_tcp_socket();
        friend basic_socket make_udp_socket();

    private:
        struct basic_socket_info {
            bool            wsa;
            int             descriptor;
            socket_domain   domain;
            socket_type     type;
            socket_protocol protocol;
        };

        std::shared_ptr<basic_socket_info> m_info_;

        unsigned short _M_resolve_service(const std::string& __service,
            const std::string& __protocol = "tcp");

        void _M_copy(const basic_socket& __rhs);
        void _M_move(basic_socket&& __rhs);
        void _M_dtor();
    };

    basic_socket make_socket(socket_domain domain, socket_type type,
        socket_protocol proto) /* throw(impact_error) */;
    basic_socket make_tcp_socket() /* throw(impact_error) */;
    basic_socket make_udp_socket() /* throw(impact_error) */;
}

#endif
