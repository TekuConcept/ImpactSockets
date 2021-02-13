/**
 * Created by TekuConcept on December 31, 2020
 */

#ifndef IMPACT_EVENT_LOOP_INTERFACE_H
#define IMPACT_EVENT_LOOP_INTERFACE_H

#include <cstdint>
#include <functional>
#include <memory>
#include "async/tcp_server_interface.h"
#include "async/tcp_client_interface.h"

namespace impact {

    // FIXME: uintptr_t type is not guaranteed to exist
    typedef std::uintptr_t etimer_id_t;
    typedef unsigned long long int etimer_time_t;
    typedef std::function<void()> etimer_callback_t;
    typedef std::shared_ptr<tcp_client_interface> tcp_client_pt;
    typedef std::shared_ptr<tcp_server_interface> tcp_server_pt;
    // typedef std::shared_ptr<udp_socket_interface> udp_socket_pt;

    class event_loop_interface {
    public:
        virtual ~event_loop_interface() = default;

        virtual void run() = 0;
        virtual void run_async() = 0;
        virtual void stop() = 0;

        virtual etimer_id_t set_timeout(etimer_callback_t cb, etimer_time_t ms) = 0;
        virtual etimer_id_t set_interval(etimer_callback_t cb, etimer_time_t ms) = 0;
        virtual etimer_id_t set_immediate(etimer_callback_t cb) = 0;
        virtual void clear_timeout(etimer_id_t id) = 0;
        virtual void clear_interval(etimer_id_t id) = 0;
        virtual void clear_immediate(etimer_id_t id) = 0;

        virtual tcp_server_pt create_tcp_server() = 0;
        virtual tcp_client_pt create_tcp_client() = 0;
        // virtual udp_socket_pt create_udp_socket() = 0;
    };

    typedef std::shared_ptr<event_loop_interface> event_loop_pt;
    event_loop_pt default_event_loop();
    void default_event_loop(event_loop_pt event_loop);

} /* namespace impact */

#endif /* IMPACT_EVENT_LOOP_INTERFACE_H */
