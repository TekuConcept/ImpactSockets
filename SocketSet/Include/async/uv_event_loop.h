/**
 * Created by TekuConcept on December 31, 2020
 */

#ifndef IMPACT_UV_EVENT_LOOP_H
#define IMPACT_UV_EVENT_LOOP_H

#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include "uv.h"
#include "async/abstract_event_loop.h"

namespace impact {

    class uv_event_loop : public abstract_event_loop {
    public:
        uv_event_loop();
        ~uv_event_loop();

        void run() override;
        void run_async() override;
        void stop() override;

        etimer_id_t set_timeout(etimer_callback_t cb, etimer_time_t ms);
        etimer_id_t set_interval(etimer_callback_t cb, etimer_time_t ms);
        etimer_id_t set_immediate(etimer_callback_t cb);
        void clear_timeout(etimer_id_t id);
        void clear_interval(etimer_id_t id);
        void clear_immediate(etimer_id_t id);

    private:
        bool              m_is_async_thread;
        std::atomic<bool> m_is_running;
        std::thread::id   m_isolate;
        std::thread       m_async_thread;
        uv_async_t        m_async_stop_handle;
        uv_async_t        m_async_call_handle;
        uv_timer_t        m_keep_alive_timer;
        uv_loop_t         m_loop;
        uv_rwlock_t       m_lock;

        struct timer_request_info {
            bool clear;
            std::shared_ptr<uv_timer_t> timer;
            etimer_id_t id;
            etimer_time_t timeout;
            etimer_time_t interval;
            etimer_callback_t callback;
        };
        std::vector<struct timer_request_info> m_timer_queue;
        std::vector<std::shared_ptr<uv_timer_t>> m_timers;

        void _M_cleanup_thread();

        etimer_id_t _M_create_timer(
            const etimer_callback_t& cb,
            etimer_time_t timeout_ms,
            etimer_time_t interval_ms,
            std::shared_ptr<uv_timer_t> timer = nullptr);
        etimer_id_t _M_create_timer_async(
            const etimer_callback_t& cb,
            etimer_time_t timeout_ms,
            etimer_time_t interval_ms);
        void _M_cleanup_timer(etimer_id_t id);
        void _M_cleanup_timer_async(etimer_id_t id);

        friend void async_uvcall_callback(uv_async_t* async);
    };

} /* namespace impact */

#endif
