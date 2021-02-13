/**
 * Created by TekuConcept on January 18, 2021
 */

#ifndef IMPACT_TCP_SERVER_H
#define IMPACT_TCP_SERVER_H

#include <memory>
#include "async/event_loop_interface.h"
#include "sockets/tcp_server_interface.h"

namespace impact {

    // FIXME: forward events from m_base to this
    class tcp_server : public tcp_server_interface {
    public:
        tcp_server(std::shared_ptr<event_loop_interface>
            event_loop = default_event_loop());
        ~tcp_server() = default;

        tcp_address_t address() const;
        bool listening() const;
        size_t max_connections() const;
        void max_connections(size_t value);

        void close(event_emitter::callback_t cb = nullptr);
        void listen(
            std::string path,
            event_emitter::callback_t cb = nullptr);
        void listen(
            unsigned short port,
            event_emitter::callback_t cb);
        void listen(
            unsigned short port = 0,
            std::string host = "0.0.0.0",
            event_emitter::callback_t cb = nullptr);

    private:
        std::shared_ptr<tcp_server_interface> m_base;
    };

} /* namespace impact */

#endif /* IMPACT_TCP_SOCKET_H */
