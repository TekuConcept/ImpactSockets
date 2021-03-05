/**
 * Created by TekuConcept on December 31, 2020
 */

#ifndef IMPACT_UV_EVENT_LOOP_H
#define IMPACT_UV_EVENT_LOOP_H

#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <future>
#include "uv.h"
#include "async/event_loop_interface.h"

namespace impact {

    class uv_event_loop : public event_loop_interface {
    public:
        uv_event_loop();
        ~uv_event_loop();

        void run() override;
        void run_async() override;
        void stop() override;

        etimer_id_t set_timeout(etimer_callback_t cb, etimer_time_t ms) override;
        etimer_id_t set_interval(etimer_callback_t cb, etimer_time_t ms) override;
        etimer_id_t set_immediate(etimer_callback_t cb) override;
        void clear_timeout(etimer_id_t id) override;
        void clear_interval(etimer_id_t id) override;
        void clear_immediate(etimer_id_t id) override;

        tcp_server_t create_tcp_server() override;
        tcp_client_t create_tcp_client() override;
        udp_socket_t create_udp_socket() override;

        void invoke(invoke_callback_t cb, bool blocking = false) override;

    private:
        enum class request_type { NONE, INVOKE };
        struct async_request_t {
            request_type type;
            invoke_callback_t invoke_callback;
            std::promise<void>* promise;

            async_request_t();
        };

        struct context_t {
            bool              is_async_thread;
            std::atomic<bool> is_running;
            std::thread::id   isolate;
            std::thread       async_thread;
            uv_async_t        async_call_handle;
            uv_timer_t        keep_alive_timer;
            uv_loop_t         loop;
            uv_rwlock_t       lock;

            std::vector<struct async_request_t> requests;
            std::vector<std::shared_ptr<uv_timer_t>> timers;
        };

        std::shared_ptr<struct context_t> m_context;

        void _M_cleanup_thread();

        etimer_id_t _M_create_timer(
            const etimer_callback_t& cb,
            etimer_time_t timeout_ms,
            etimer_time_t interval_ms,
            std::shared_ptr<uv_timer_t> timer = nullptr);
        void _M_cleanup_timer(etimer_id_t id);

        friend void async_uvcall_callback(uv_async_t* async);
        friend class uv_tcp_server;
        friend class uv_tcp_client;
        friend class uv_udp_socket;
    };

} /* namespace impact */

#endif
