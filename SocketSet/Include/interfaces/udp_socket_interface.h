/**
 * Created by TekuConcept on March 4, 2021
 */

#ifndef IMPACT_UDP_SOCKET_INTERFACE_H
#define IMPACT_UDP_SOCKET_INTERFACE_H

#include <string>
#include <functional>
#include "utils/event_emitter.h"
#include "sockets/types.h"

namespace impact {

    struct udp_address_t {
        unsigned short port;
        address_family family;
        std::string address;
    };

    class udp_socket_observer_interface {
    public:
        virtual ~udp_socket_observer_interface() = default;
        virtual void on_close() = 0;
        virtual void on_connect() = 0;
        virtual void on_error(const std::string& message) = 0;
        virtual void on_listening() = 0;
        virtual void on_message(
            const std::string& data,
            const udp_address_t& address) = 0;
    };

    class udp_socket_interface : public event_emitter {
    public:
        virtual ~udp_socket_interface() = default;

        virtual int descriptor() const = 0;

        virtual udp_address_t address() const = 0;
        virtual udp_address_t remote_address() const = 0;
        // virtual size_t recv_buffer_size() const = 0;
        // virtual size_t send_buffer_size() const = 0;
        // virtual void recv_buffer_size(size_t size) = 0;
        // virtual void send_buffer_size(size_t size) = 0;

        virtual void add_membership(
            std::string address,
            std::string interface = std::string()) = 0;
        virtual void drop_membership(
            std::string address,
            std::string interface = std::string()) = 0;
        // virtual void add_source_specific_membership(
        //     std::string source_address,
        //     std::string group_address,
        //     std::string interface = std::string()) = 0;
        // virtual void drop_source_specific_membership(
        //     std::string source_address,
        //     std::string group_address,
        //     std::string interface = std::string()) = 0;

        virtual void bind(
            unsigned short port = 0,
            std::string address = "127.0.0.1",
            event_emitter::callback_t cb = nullptr) = 0;
        inline void bind(
            unsigned short port,
            event_emitter::callback_t cb)
        { bind(port, "127.0.0.1", cb); }
        virtual void close(event_emitter::callback_t cb = nullptr) = 0;
        // virtual void connect(
        //     unsigned short port = 0,
        //     std::string address = "127.0.0.1",
        //     event_emitter::callback_t cb = nullptr) = 0;
        // inline void connect(
        //     unsigned short port,
        //     event_emitter::callback_t cb)
        // { connect(port, "127.0.0.1", cb); }
        // virtual void disconnect() = 0;

        virtual void send(
            std::string message,
            size_t offset,
            size_t length,
            unsigned short port = 0,
            std::string address = "127.0.0.1",
            event_emitter::callback_t cb = nullptr) = 0;
        inline void send(
            std::string message,
            unsigned short port = 0,
            std::string address = "127.0.0.1",
            event_emitter::callback_t cb = nullptr)
            { send(message, 0, std::string::npos, port, address, cb); }
        inline void send(
            std::string message,
            std::string address,
            event_emitter::callback_t cb = nullptr)
            { send(message, 0, std::string::npos, 0, address, cb); }
        inline void send(
            std::string message,
            event_emitter::callback_t cb)
            { send(message, 0, std::string::npos, 0, "127.0.0.1", cb); }

        virtual void pause() = 0;
        virtual void resume() = 0;

        virtual void set_broadcast(bool flag) = 0;
        virtual void set_multicast_interface(std::string interface) = 0;
        virtual void set_multicast_loopback(bool flag) = 0;
        virtual void set_multicast_ttl(unsigned char time_to_live) = 0;
        virtual void set_ttl(unsigned char time_to_live) = 0;

        virtual void set_event_observer(udp_socket_observer_interface*) = 0;
    };

    typedef std::shared_ptr<udp_socket_interface> udp_socket_t;

} /* namespace impact */

#endif /* IMPACT_UDP_SOCKET_INTERFACE_H */
