/**
 * Created by TekuConcept on January 18, 2021
 */

#ifndef IMPACT_TCP_SERVER_INTERFACE_H
#define IMPACT_TCP_SERVER_INTERFACE_H

#include <memory>
#include <functional>
#include "utils/event_emitter.h"
#include "interfaces/tcp_client_interface.h"

namespace impact {

    class tcp_server_observer_interface {
    public:
        virtual ~tcp_server_observer_interface() = default;
        virtual void on_close() = 0;
        virtual void on_connection(const tcp_client_t& connection) = 0;
        virtual void on_error(const std::string& message) = 0;
        virtual void on_listening() = 0;
    };

    class tcp_client_interface;
    class tcp_server_interface : public event_emitter {
    public:
        virtual ~tcp_server_interface() = default;

        virtual tcp_address_t address() const = 0;
        virtual bool listening() const = 0;
        virtual size_t max_connections() const = 0;
        virtual void max_connections(size_t value) = 0;

        virtual void close(event_emitter::callback_t cb = nullptr) = 0;
        virtual void listen(
            std::string path,
            event_emitter::callback_t cb = nullptr) = 0;
        virtual void listen(
            unsigned short port = 0,
            std::string host = "127.0.0.1",
            event_emitter::callback_t cb = nullptr) = 0;
        inline void listen(
            unsigned short port,
            event_emitter::callback_t cb)
        { listen(port, "127.0.0.1", cb); }

        virtual void set_event_observer(tcp_server_observer_interface*) = 0;
    };

    typedef std::shared_ptr<tcp_server_interface> tcp_server_t;

} /* namespace impact */

#endif /* IMPACT_TCP_SERVER_INTERFACE_H */
