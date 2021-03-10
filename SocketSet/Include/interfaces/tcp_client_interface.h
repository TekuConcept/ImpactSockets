/**
 * Created by TekuConcept on January 18, 2021
 */

#ifndef IMPACT_TCP_CLIENT_INTERFACE_H
#define IMPACT_TCP_CLIENT_INTERFACE_H

#include <string>
#include <memory>
#include <functional>
#include "sockets/types.h"
#include "utils/event_emitter.h"

namespace impact {

    struct tcp_address_t {
        unsigned short port;
        address_family family;
        std::string address;
    };

    class tcp_client_observer_interface {
    public:
        virtual ~tcp_client_observer_interface() = default;
        virtual void on_close(bool transmission_error) = 0;
        virtual void on_connect() = 0;
        virtual void on_data(std::string& data) = 0;
        virtual void on_end() = 0;
        virtual void on_error(const std::string& message) = 0;
        virtual void on_lookup(
            std::string& error,
            std::string& address,
            address_family family,
            std::string& host) = 0;
        virtual void on_ready() = 0;
        virtual void on_timeout() = 0;
    };

    class tcp_client_interface : public event_emitter {
    public:
        enum class ready_state_t {
            PENDING,
            OPENING,
            OPEN,
            READ_ONLY,
            WRITE_ONLY,
            DESTROYED
        };

        virtual ~tcp_client_interface() = default;

        virtual tcp_address_t address() const = 0;
        virtual size_t bytes_read() const = 0;
        virtual size_t bytes_written() const = 0;
        virtual bool connecting() const = 0;
        virtual bool destroyed() const = 0;
        virtual std::string local_address() const = 0;
        virtual unsigned short local_port() const = 0;
        virtual bool pending() const = 0;
        virtual std::string remote_address() const = 0;
        virtual address_family remote_family() const = 0;
        virtual unsigned short remote_port() const = 0;
        virtual size_t timeout() const = 0;
        virtual ready_state_t ready_state() const = 0;

        virtual tcp_client_interface* connect(
            std::string path,
            event_emitter::callback_t cb = nullptr) = 0;
        virtual tcp_client_interface* connect(
            unsigned short port,
            std::string host = "127.0.0.1",
            event_emitter::callback_t cb = nullptr) = 0;
        inline tcp_client_interface* connect(
            unsigned short port,
            event_emitter::callback_t cb)
        { return connect(port, "127.0.0.1", cb); }
        virtual tcp_client_interface* destroy(std::string error = "") = 0;
        virtual tcp_client_interface* end(
            std::string data = std::string(),
            std::string encoding = "utf8",
            event_emitter::callback_t cb = nullptr) = 0;
        inline tcp_client_interface* end(
            std::string data,
            event_emitter::callback_t cb)
        { return end(data, "utf8", cb); }
        inline tcp_client_interface* end(event_emitter::callback_t cb)
        { return end(std::string(), "utf8", cb); }
        virtual tcp_client_interface* pause() = 0;
        virtual tcp_client_interface* resume() = 0;
        virtual tcp_client_interface* set_encoding(
            std::string encoding = "utf8") = 0;
        virtual tcp_client_interface* set_keep_alive(
            bool enable = false,
            unsigned int initial_delay = 0) = 0;
        inline tcp_client_interface* set_keep_alive(
            unsigned int initial_delay)
        { return set_keep_alive(false, initial_delay); }
        virtual tcp_client_interface* set_no_delay(
            bool no_delay = true) = 0;
        virtual tcp_client_interface* set_timeout(
            unsigned int timeout,
            event_emitter::callback_t cb = nullptr) = 0;
        virtual bool write(
            std::string data,
            std::string encoding = "utf8",
            event_emitter::callback_t cb = nullptr) = 0;
        inline bool write(
            std::string data,
            event_emitter::callback_t cb)
        { return write(data, "utf8", cb); }

        virtual void set_event_observer(tcp_client_observer_interface*) = 0;
    };

    typedef std::shared_ptr<tcp_client_interface> tcp_client_t;

} /* namespace impact */

#endif /* IMPACT_TCP_CLIENT_INTERFACE_H */
