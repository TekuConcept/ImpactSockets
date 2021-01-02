/**
 * Created by TekuConcept on December 31, 2020
 */

#include <iostream>
#include <chrono>
#include <algorithm>
#include <future>
#include "async/uv_event_loop.h"
#include "utils/environment.h"
#include "utils/impact_error.h"

using namespace impact;


static void
async_stop_callback(uv_async_t* async)
{
    uv_loop_t* loop = static_cast<uv_loop_t*>(async->data);
    uv_stop(loop);
}


namespace impact {
    void
    async_uvcall_callback(uv_async_t* async)
    {
        uv_event_loop* event_loop = reinterpret_cast<uv_event_loop*>(async->data);
        std::vector<struct uv_event_loop::timer_request_info> queue;

        // make sure we are running on the event-loop thread
        if (std::this_thread::get_id() != event_loop->m_isolate) return;

        // get requests to add or remove timers
        uv_rwlock_wrlock(&event_loop->m_lock);
        queue = std::move(event_loop->m_timer_queue);
        uv_rwlock_wrunlock(&event_loop->m_lock);

        // now process each request
        for (const auto& request : queue) {
            if (request.clear) event_loop->_M_cleanup_timer(request.id);
            else event_loop->_M_create_timer(
                request.callback,
                request.timeout,
                request.interval,
                request.timer);
        }
    }
} /* namespace impact */


static void
keep_alive_callback(uv_timer_t* /*handle*/)
{
    // this is just used to keep the loop running;
    // - do nothing for now -
}


static void
timer_invoke_callback(uv_timer_t* handle)
{
    if (handle->data == nullptr) return; // precaution
    etimer_callback_t* cb = reinterpret_cast<etimer_callback_t*>(handle->data);
    (*cb)();
}


uv_event_loop::uv_event_loop()
: m_is_async_thread(false),
  m_is_running(false)
{
    m_isolate = std::this_thread::get_id();
    uv_rwlock_init(&m_lock);

    uv_loop_init(&m_loop);
    uv_async_init(&m_loop, &m_async_stop_handle, async_stop_callback);
    uv_async_init(&m_loop, &m_async_call_handle, async_uvcall_callback);
    uv_timer_init(&m_loop, &m_keep_alive_timer);

    m_async_stop_handle.data = &m_loop;
    m_async_call_handle.data = this;

    uv_timer_start(
        &m_keep_alive_timer,
        keep_alive_callback,
        /*timeout=*/5000,
        /*repeat=*/5000);
}


uv_event_loop::~uv_event_loop()
{
    stop(); // invocation does nothing if not already running

    uv_timer_stop(&m_keep_alive_timer);
    uv_close(reinterpret_cast<uv_handle_t*>(&m_keep_alive_timer), nullptr);
    uv_close(reinterpret_cast<uv_handle_t*>(&m_async_stop_handle), nullptr);

    switch (uv_loop_close(&m_loop)) {
        case 0:            /* loop closed           */ break;
        case UV_ECANCELED: /* cancelled             */ break;
        case UV_EBUSY:     /* busy                  */ break;
        default:           /* unexpected error code */ break;
    }

    uv_rwlock_destroy(&m_lock);
}


void
uv_event_loop::run()
{
    if (m_is_running) return;

    _M_cleanup_thread();
    m_timer_queue.clear(); // precaution - reset

    m_isolate = std::this_thread::get_id();
    m_is_running = true;
    uv_run(&m_loop, UV_RUN_DEFAULT);
    m_is_running = false;
}


void
uv_event_loop::run_async()
{
    if (m_is_running) return;

    _M_cleanup_thread();
    m_timer_queue.clear(); // precaution - reset

    m_is_running = true;
    m_is_async_thread = true;

    std::promise<void> wait_promise;
    std::future<void> wait = wait_promise.get_future();
    m_async_thread = std::thread([&]() {
        m_isolate = std::this_thread::get_id();
        wait_promise.set_value();
        uv_run(&m_loop, UV_RUN_DEFAULT);
        m_is_running = false;
    });
    wait.get(); // wait for thread to start before returning
}


void
uv_event_loop::stop()
{
    if (std::this_thread::get_id() == m_isolate) {
        // do not attempt to join thread to itself
        // also skip the async request to stop
        uv_stop(&m_loop);
    }
    else {
        if (m_is_running)
            uv_async_send(&m_async_stop_handle);
        _M_cleanup_thread();
    }
}


void
uv_event_loop::_M_cleanup_thread()
{
    if (m_is_async_thread) {
        if (m_async_thread.joinable())
            m_async_thread.join();
        m_is_async_thread = false;
        m_isolate = std::this_thread::get_id();
    }
}


etimer_id_t
uv_event_loop::set_timeout(
    etimer_callback_t __cb,
    etimer_time_t     __ms)
{
    if (std::this_thread::get_id() == m_isolate)
        return _M_create_timer(__cb, __ms, 0);
    else return _M_create_timer_async(__cb, __ms, 0);
}


etimer_id_t
uv_event_loop::set_interval(
    etimer_callback_t __cb,
    etimer_time_t     __ms)
{
    if (std::this_thread::get_id() == m_isolate)
        return _M_create_timer(__cb, __ms, __ms);
    else return _M_create_timer_async(__cb, __ms, __ms);
}


etimer_id_t
uv_event_loop::set_immediate(etimer_callback_t __cb)
{
    if (std::this_thread::get_id() == m_isolate)
        return _M_create_timer(__cb, 0, 0);
    else return _M_create_timer_async(__cb, 0, 0);
}


void
uv_event_loop::clear_timeout(etimer_id_t __id)
{
    if (std::this_thread::get_id() == m_isolate)
        _M_cleanup_timer(__id);
    else _M_cleanup_timer_async(__id);
}


void
uv_event_loop::clear_interval(etimer_id_t __id)
{
    if (std::this_thread::get_id() == m_isolate)
        _M_cleanup_timer(__id);
    else _M_cleanup_timer_async(__id);
}


void
uv_event_loop::clear_immediate(etimer_id_t __id)
{
    if (std::this_thread::get_id() == m_isolate)
        _M_cleanup_timer(__id);
    else _M_cleanup_timer_async(__id);
}


etimer_id_t
uv_event_loop::_M_create_timer(
    const etimer_callback_t&    __cb,
    etimer_time_t               __timeout_ms,
    etimer_time_t               __interval_ms,
    std::shared_ptr<uv_timer_t> __timer)
{
    std::shared_ptr<uv_timer_t> timer = __timer;
    if (timer == nullptr) timer = std::make_shared<uv_timer_t>();

    uv_timer_init(&m_loop, timer.get());

    timer->data = new etimer_callback_t([&, __cb, __interval_ms, timer]() {
        __cb();
        if (__interval_ms == 0)
            _M_cleanup_timer(reinterpret_cast<etimer_id_t>(timer.get()));
    });

    uv_timer_start(
        timer.get(),
        timer_invoke_callback,
        __timeout_ms,
        __interval_ms);

    m_timers.push_back(timer);
    return reinterpret_cast<etimer_id_t>(timer.get());
}


etimer_id_t
uv_event_loop::_M_create_timer_async(
    const etimer_callback_t& __cb,
    etimer_time_t            __timeout_ms,
    etimer_time_t            __interval_ms)
{
    struct timer_request_info info;
    info.clear    = false;
    info.callback = __cb;
    info.timeout  = __timeout_ms;
    info.interval = __interval_ms;
    info.timer    = std::make_shared<uv_timer_t>();
    info.id       = 0;

    uv_rwlock_wrlock(&m_lock);
    m_timer_queue.push_back(info);
    uv_rwlock_wrunlock(&m_lock);
    uv_async_send(&m_async_call_handle);

    return reinterpret_cast<etimer_id_t>(info.timer.get());
}


void
uv_event_loop::_M_cleanup_timer(etimer_id_t __id)
{
    auto res = std::find_if(m_timers.begin(), m_timers.end(),
    [=](const std::shared_ptr<uv_timer_t>& timer) {
        return reinterpret_cast<etimer_id_t>(timer.get()) == __id;
    });
    // timer dosen't exist; ignore
    if (res == m_timers.end()) return;
    // deref timer and trigger pointer cleanup
    delete reinterpret_cast<etimer_callback_t*>((*res)->data);
    (*res)->data = nullptr;
    uv_timer_stop(res->get());
    uv_close(reinterpret_cast<uv_handle_t*>(res->get()), nullptr);
    m_timers.erase(res);
}


void
uv_event_loop::_M_cleanup_timer_async(etimer_id_t __id)
{
    uv_rwlock_wrlock(&m_lock);

    auto res = std::find_if(m_timer_queue.begin(), m_timer_queue.end(),
    [&](const struct timer_request_info& timer_info) {
        return reinterpret_cast<etimer_id_t>(timer_info.timer.get()) == __id;
    });

    // do not signal the event-loop if we don't have to
    if (res != m_timer_queue.end()) {
        m_timer_queue.erase(res);
        uv_rwlock_wrunlock(&m_lock);
    }
    else {
        struct timer_request_info info;
        info.clear    = true;
        info.callback = nullptr;
        info.timeout  = 0;
        info.interval = 0;
        info.timer    = nullptr;
        info.id       = __id;
        m_timer_queue.push_back(info);
        uv_rwlock_wrunlock(&m_lock);
        uv_async_send(&m_async_call_handle);
    }
}
