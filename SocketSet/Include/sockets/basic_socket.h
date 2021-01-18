/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef IMPACT_BASIC_SOCKET_H
#define IMPACT_BASIC_SOCKET_H

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
        basic_socket(address_family domain, socket_type type, internet_protocol proto);
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
        long use_count()             const noexcept;
        int get()                    const noexcept;
        address_family domain()      const noexcept;
        socket_type type()           const noexcept;
        internet_protocol protocol() const noexcept;
        explicit operator bool()     const noexcept;

        // communication / delivery
        void bind(unsigned short port) /* throw(impact_error) */;
        void bind(std::string address, unsigned short port = 0)
            /* throw(impact_error) */;
        void bind(const struct sockaddr& address)
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

        static basic_socket create_tcp() /* throw(impact_error) */;
        static basic_socket create_udp() /* throw(impact_error) */;

    private:
        struct basic_socket_info {
            bool              wsa;
            int               descriptor;
            address_family    domain;
            socket_type       type;
            internet_protocol protocol;
        };

        std::shared_ptr<basic_socket_info> m_info_;

        unsigned short _M_resolve_service(const std::string& __service,
            const std::string& __protocol = "tcp");

        void _M_copy(const basic_socket& __rhs);
        void _M_move(basic_socket&& __rhs);
        void _M_dtor();
    };

}

#endif
