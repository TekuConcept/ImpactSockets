/**
 * Created by TekuConcept on February 24, 2020
 */

#include "async/event_loop.h"

#include <vector>
#include <queue>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>

#include "utils/impact_error.h"

using namespace impact;


namespace impact {
namespace {
    struct idle_event {
        void* id;
        std::atomic<bool> busy;
        std::function<void(size_t)> event;
    };

    struct next_event {
        std::atomic<bool>* busy;
        std::function<void(size_t)> event;
    };

    class event_loop_impl : public event_loop_base {
    public:
        event_loop_impl(size_t poolsize = 1);
        ~event_loop_impl();

        bool add_idle_event(
            std::function<void(size_t, void*)> function,
            void* function_id);
        bool remove_idle_event(void* function_id);
        void push_event(std::function<void(size_t)> function);

    private:
        std::atomic<bool> m_shutdown_;
        std::mutex m_mtx_;
        std::condition_variable m_cv_;
        std::vector<std::thread> m_thread_pool_;

        std::vector<std::shared_ptr<struct idle_event>> m_idle_events_;
        std::queue<std::function<void(size_t)>> m_events_;
        std::atomic<bool> m_idle_pending_;

        void _M_thread_loop(size_t id);
        inline void _M_get_next_event(struct next_event* token);
    };
}}


event_loop::event_loop(size_t __poolsize)
: m_base_(new event_loop_impl(__poolsize))
{ }


event_loop::event_loop(std::shared_ptr<event_loop_base> __base)
: m_base_(__base)
{
    if (m_base_ == nullptr)
        m_base_ = std::make_shared<event_loop_impl>();
}


event_loop_impl::event_loop_impl(size_t __poolsize)
: m_shutdown_(false), m_idle_pending_(false)
{
    if (__poolsize == 0)
        throw impact_error("poolsize must be greater than zero");
    for (size_t i = 0; i < __poolsize; i++)
        m_thread_pool_.push_back(std::thread(
            &event_loop_impl::_M_thread_loop, this, i));
}


event_loop_impl::~event_loop_impl()
{
    {
        std::lock_guard<std::mutex> lock(m_mtx_);
        m_shutdown_ = true;
    }
    m_cv_.notify_all();
    for (auto& thread : m_thread_pool_) {
        if (thread.joinable())
            thread.join();
    }
}


void
event_loop_impl::_M_thread_loop(size_t __id)
{
    struct next_event token;
    do {
        _M_get_next_event(&token);
        if (token.event == nullptr) {
            std::unique_lock<std::mutex> lock(m_mtx_);
            m_cv_.wait(lock, [&]()->bool {
                return m_shutdown_ ||
                    (m_events_.size() > 0) ||
                    m_idle_pending_;
            });
            lock.unlock();
        }
        else {
            token.event(__id);
            if (token.busy != nullptr) {
                token.busy->store(false);
                // prevent thread from spinning out
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                // allow other idle threads to work on idle-events
                m_cv_.notify_one();
            }
        }
    } while (!m_shutdown_.load());
}


bool
event_loop_impl::add_idle_event(
    std::function<void(size_t, void*)> __function,
    void*                              __function_id)
{
    {
        std::lock_guard<std::mutex> lock(m_mtx_);
        auto token = std::find_if(m_idle_events_.begin(), m_idle_events_.end(),
            [&](const std::shared_ptr<struct idle_event>& ev)
            { return ev->id == __function_id; });
        if (token != m_idle_events_.end()) return false;
        std::shared_ptr<struct idle_event> ev =
            std::make_shared<struct idle_event>();
        ev->id    = __function_id;
        ev->busy  = false;
        ev->event = std::bind(__function,
            std::placeholders::_1, __function_id);
        m_idle_events_.push_back(ev);
        m_idle_pending_ = true;
    }
    m_cv_.notify_all();
    return true;
}


bool
event_loop_impl::remove_idle_event(void* __function_id)
{
    std::lock_guard<std::mutex> lock(m_mtx_);
    auto token = std::find_if(m_idle_events_.begin(), m_idle_events_.end(),
        [&](const std::shared_ptr<struct idle_event>& ev)
        { return ev->id == __function_id; });
    if (token == m_idle_events_.end())
        return false;
    m_idle_events_.erase(token);
    if (m_idle_events_.size() == 0)
        m_idle_pending_ = false;
    return true;
}


void
event_loop_impl::push_event(std::function<void(size_t)> __function)
{
    {
        std::lock_guard<std::mutex> lock(m_mtx_);
        m_events_.push(__function);
    }
    m_cv_.notify_one();
}


inline void
event_loop_impl::_M_get_next_event(struct next_event* __token)
{
    std::lock_guard<std::mutex> lock(m_mtx_);

    __token->busy  = nullptr;
    __token->event = nullptr;

    if (m_events_.size() > 0) {
        __token->event = m_events_.front();
        m_events_.pop();
    }
    else if (m_idle_events_.size() > 0) {
        size_t i;
        for (i = 0; i < m_idle_events_.size(); i++) {
            if (m_idle_events_[i]->busy) continue;
            m_idle_events_[i]->busy = true;
            __token->busy  = &m_idle_events_[i]->busy;
            __token->event = m_idle_events_[i]->event;
            if (m_idle_events_.size() > 1) {
                std::rotate(
                    m_idle_events_.begin(),
                    m_idle_events_.begin() + i + 1,
                    m_idle_events_.end());
            }
            break;
        }
        m_idle_pending_ = !(i == m_idle_pending_);
    }
}
