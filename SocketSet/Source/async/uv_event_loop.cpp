/**
 * Created by TekuConcept on December 31, 2020
 */

#include <iostream>
#include <chrono>
#include <algorithm>
#include <future>

#include "async/uv_event_loop.h"
#include "async/uv_tcp_server.h"
#include "async/uv_tcp_client.h"
#include "async/uv_udp_socket.h"
#include "utils/environment.h"
#include "utils/impact_error.h"

using namespace impact;

#define V(x) std::cout << x << std::endl

namespace impact {

    void
    async_uvcall_callback(uv_async_t* async)
    {
        uv_event_loop* event_loop = reinterpret_cast<uv_event_loop*>(async->data);
        std::vector<struct uv_event_loop::async_request_t> queue;

        // make sure we are running on the event-loop thread
        if (std::this_thread::get_id() != event_loop->m_context->isolate) return;

        // get requests to add or remove timers
        uv_rwlock_wrlock(&event_loop->m_context->lock);
        queue = std::move(event_loop->m_context->requests);
        uv_rwlock_wrunlock(&event_loop->m_context->lock);

        // now process each request
        using request_type = uv_event_loop::request_type;
        for (const auto& request : queue) {
            switch (request.type) {
            case request_type::NONE: break;
            case request_type::INVOKE: request.invoke_callback(); break;
            }
            if (request.promise) request.promise->set_value();
        }
    }

} /* namespace impact */


static void
keep_alive_callback(uv_timer_t* /*handle*/)
{ /* this is just used to keep the loop running */ }


static void
timer_invoke_callback(uv_timer_t* handle)
{
    if (handle->data == nullptr) return; // precaution
    etimer_callback_t* cb = reinterpret_cast<etimer_callback_t*>(handle->data);
    (*cb)();
}


uv_event_loop::async_request_t::async_request_t()
: type(request_type::NONE), invoke_callback(nullptr), promise(nullptr)
{ }


uv_event_loop::uv_event_loop()
{
    m_context = std::make_shared<struct context_t>();
    m_context->is_async_thread = false;
    m_context->is_running      = false;
    m_context->isolate         = std::this_thread::get_id();

    uv_rwlock_init(&m_context->lock);
    uv_loop_init(&m_context->loop);
}


uv_event_loop::~uv_event_loop()
{
    stop();
    uv_rwlock_destroy(&m_context->lock);
}


void
uv_event_loop::run()
{
    if (m_context->is_running) return;

    _M_cleanup_thread();
    m_context->requests.clear(); // precaution - reset

    m_context->isolate = std::this_thread::get_id();
    m_context->is_running = true;

    uv_async_init(
        &m_context->loop,
        &m_context->async_call_handle,
        async_uvcall_callback);
    uv_timer_init(&m_context->loop, &m_context->keep_alive_timer);
    m_context->async_call_handle.data = this;

    uv_timer_start(
        &m_context->keep_alive_timer,
        keep_alive_callback,
        /*timeout=*/500,
        /*repeat=*/500);

    uv_run(&m_context->loop, UV_RUN_DEFAULT);
    m_context->is_running = false;
}


void
uv_event_loop::run_async()
{
    if (m_context->is_running) return;

    _M_cleanup_thread();
    m_context->requests.clear(); // precaution - reset

    m_context->is_running = true;
    m_context->is_async_thread = true;

    uv_async_init(
        &m_context->loop,
        &m_context->async_call_handle,
        async_uvcall_callback);
    uv_timer_init(&m_context->loop, &m_context->keep_alive_timer);
    m_context->async_call_handle.data = this;

    uv_timer_start(
        &m_context->keep_alive_timer,
        keep_alive_callback,
        /*timeout=*/500,
        /*repeat=*/500);

    std::promise<void> wait_promise;
    std::future<void> wait = wait_promise.get_future();
    m_context->async_thread = std::thread([&]() {
        m_context->isolate = std::this_thread::get_id();
        wait_promise.set_value();
        uv_run(&m_context->loop, UV_RUN_DEFAULT);
        m_context->is_running = false;
    });
    wait.get(); // wait for thread to start before returning
}


void
uv_event_loop::stop()
{
    _M_blocking_stop();
    if (std::this_thread::get_id() != m_context->isolate)
        _M_cleanup_thread();
}


void
uv_event_loop::_M_blocking_stop()
{
    if (m_context->is_running) {
        {
            std::lock_guard<std::mutex> lock(m_context->child_signal_mtx);
            for (const auto& child : m_context->children)
                child->send_signal(uv_node_signal_t::STOP);
        }

        this->invoke([&]() {
            uv_timer_stop(&m_context->keep_alive_timer);
            if (!uv_is_closing((uv_handle_t*)&m_context->keep_alive_timer))
                uv_close(reinterpret_cast<uv_handle_t*>
                    (&m_context->keep_alive_timer), nullptr);
            if (!uv_is_closing((uv_handle_t*)&m_context->async_call_handle))
                uv_close(reinterpret_cast<uv_handle_t*>
                    (&m_context->async_call_handle), nullptr);
            uv_stop(&m_context->loop);
        }, /*blocking=*/true);

        int result;
        do {
            result = uv_loop_close(&m_context->loop);
            switch (result) {
                case 0:            /* loop closed           */ break;
                case UV_ECANCELED: /* cancelled             */ break;
                case UV_EBUSY:     /* busy                  */ break;
                default:           /* unexpected error code */ break;
            }
            // try again in 250ms
            // WARNING: possible infinite loop
            // NOTE: returning from dtor without successfully closing
            //       the loop will result in a memory leak
            if (result != 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
        } while (result != 0);
        m_context->is_running = false;
    }

    // join async threads
    if (std::this_thread::get_id() != m_context->isolate)
        _M_cleanup_thread();
}


void
uv_event_loop::_M_cleanup_thread()
{
    if (m_context->is_async_thread) {
        if (m_context->async_thread.joinable())
            m_context->async_thread.join();
        m_context->is_async_thread = false;
        m_context->isolate = std::this_thread::get_id();
    }
}


etimer_id_t
uv_event_loop::set_timeout(
    etimer_callback_t __cb,
    etimer_time_t     __ms)
{
    etimer_id_t id;
    this->invoke([this, __cb, __ms, &id]() {
        id = _M_create_timer(__cb, __ms, 0);
    }, /*blocking=*/true);
    return id;
}


etimer_id_t
uv_event_loop::set_interval(
    etimer_callback_t __cb,
    etimer_time_t     __ms)
{
    etimer_id_t id;
    this->invoke([this, __cb, __ms, &id]() {
        id = _M_create_timer(__cb, __ms, __ms);
    }, /*blocking=*/true);
    return id;
}


etimer_id_t
uv_event_loop::set_immediate(etimer_callback_t __cb)
{
    etimer_id_t id;
    this->invoke([this, __cb, &id]() {
        id = _M_create_timer(__cb, 0, 0);
    }, /*blocking=true*/true);
    return id;
}


void
uv_event_loop::clear_timeout(etimer_id_t __id)
{ this->invoke([this, __id]() { _M_cleanup_timer(__id); }); }


void
uv_event_loop::clear_interval(etimer_id_t __id)
{ this->invoke([this, __id]() { _M_cleanup_timer(__id); }); }


void
uv_event_loop::clear_immediate(etimer_id_t __id)
{ this->invoke([this, __id]() { _M_cleanup_timer(__id); }); }


tcp_server_t
uv_event_loop::create_tcp_server()
{ return tcp_server_t(new uv_tcp_server(this)); }


tcp_client_t
uv_event_loop::create_tcp_client()
{ return tcp_client_t(new uv_tcp_client(this)); }


udp_socket_t
uv_event_loop::create_udp_socket()
{ return udp_socket_t(new uv_udp_socket(this)); }


void
uv_event_loop::invoke(
    invoke_callback_t __cb,
    bool              __blocking)
{
    if (!__cb) return;
    if (std::this_thread::get_id() == m_context->isolate) __cb();
    else {
        struct async_request_t request;
        request.type            = request_type::INVOKE;
        request.invoke_callback = __cb;

        std::promise<void> p;
        std::future<void> f;
        if (__blocking) {
            request.promise = &p;
            f = p.get_future();
        }

        uv_rwlock_wrlock(&m_context->lock);
        m_context->requests.push_back(request);
        uv_rwlock_wrunlock(&m_context->lock);
        uv_async_send(&m_context->async_call_handle);

        if (__blocking) try { f.get(); } catch (...) { /* ignore */ }
    }
}


void
uv_event_loop::add_child(uv_child_interface* __child)
{
    if (__child == nullptr) return;
    std::lock_guard<std::mutex> lock(m_context->child_signal_mtx);
    m_context->children.push_back(__child);
}


void
uv_event_loop::remove_child(uv_child_interface* __child)
{
    std::lock_guard<std::mutex> lock(m_context->child_signal_mtx);
    auto child = std::find(
        m_context->children.begin(),
        m_context->children.end(),
        __child
    );
    if (child == m_context->children.end()) return;
    m_context->children.erase(child);
}


uv_loop_t*
uv_event_loop::get_loop_handle() const
{ return &m_context->loop; }


etimer_id_t
uv_event_loop::_M_create_timer(
    const etimer_callback_t&    __cb,
    etimer_time_t               __timeout_ms,
    etimer_time_t               __interval_ms,
    std::shared_ptr<uv_timer_t> __timer)
{
    std::shared_ptr<uv_timer_t> timer = __timer;
    if (timer == nullptr) timer = std::make_shared<uv_timer_t>();

    uv_timer_init(&m_context->loop, timer.get());

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

    m_context->timers.push_back(timer);
    return reinterpret_cast<etimer_id_t>(timer.get());
}


void
uv_event_loop::_M_cleanup_timer(etimer_id_t __id)
{
    auto res = std::find_if(m_context->timers.begin(), m_context->timers.end(),
    [=](const std::shared_ptr<uv_timer_t>& timer) {
        return reinterpret_cast<etimer_id_t>(timer.get()) == __id;
    });
    // timer dosen't exist; ignore
    if (res == m_context->timers.end()) return;
    // deref timer and trigger pointer cleanup
    delete reinterpret_cast<etimer_callback_t*>((*res)->data);
    (*res)->data = nullptr;
    uv_timer_stop(res->get());
    uv_close(reinterpret_cast<uv_handle_t*>(res->get()), nullptr);
    m_context->timers.erase(res);
}
