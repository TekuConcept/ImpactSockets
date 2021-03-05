/**
 * Created by TekuConcept on February 13, 2021
 */

#ifndef IMPACT_ASYNC_SOCKET_H
#define IMPACT_ASYNC_SOCKET_H

#include <memory>
#include "async/tcp_client_interface.h"
#include "async/tcp_server_interface.h"
#include "async/event_loop_interface.h"

namespace impact {

    class async_socket {
    public:
        async_socket() = delete;

        static tcp_client_t create_tcp_client(
            event_loop_t event_loop = default_event_loop());
        static tcp_server_t create_tcp_server(
            event_loop_t event_loop = default_event_loop());
        // static udp_socket_t create_udp_socket(
        //     event_loop_t event_loop = default_event_loop());
    };

} /* namespace impact */

#endif /* IMPACT_ASYNC_SOCKET_H */
