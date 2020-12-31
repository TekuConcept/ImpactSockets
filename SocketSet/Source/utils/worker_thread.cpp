/**
 * Created by TekuConcept on July 25, 2018
 */

#include "utils/worker_thread.h"

using namespace impact;
using namespace internal;

worker_thread::worker_thread()
{
    _M_begin();
}


worker_thread::~worker_thread()
{
    _M_end();
}


void
worker_thread::_M_begin()
{
    m_shutting_down_ = false;
    m_thread_        = std::thread([&](){
        do {
            std::unique_lock<std::mutex> lock(m_thread_mtx_);
            m_thread_cv_.wait(lock, [&]() -> bool {
                return m_shutting_down_ || _M_has_work();
            });
            if (m_shutting_down_) break;
            /* avoid deadlock with _M_locked_run() and _M_end() */
            lock.unlock();
            _M_dowork();
        } while (!m_shutting_down_);
    });
}


void
worker_thread::_M_end()
{
    {
        std::lock_guard<std::mutex> lock(m_thread_mtx_);
        m_shutting_down_ = true;
    }
    m_thread_cv_.notify_one();
    if (m_thread_.joinable())
        m_thread_.join();
}


bool
worker_thread::_M_has_work()
{
    return false;
}


void
worker_thread::_M_dowork()
{ /* do nothing */ }


void
worker_thread::_M_register_obj(object_ptr __obj)
{
    /* not thread safe */
    m_objects_.push_back(__obj);
}


void
worker_thread::_M_locked_run(functor* __func)
{
    if (__func) {
        std::lock_guard<std::mutex> lock(m_thread_mtx_);
        __func->callback();
    }
}
