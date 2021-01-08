/**
 * Created by TekuConcept on January 5, 2021
 */

#include <iostream>
#include "async/promise.h"

using namespace impact;


promise::link_t::link_t()
: state_id(state_t::PENDING)
{ }


promise::promise()
: m_link(std::make_shared<struct link_t>())
{ }


promise::promise(std::shared_ptr<struct link_t> __link)
: m_link(__link)
{ }


inline promise::callback_t
promise::_M_wrap_callback(
    const std::shared_ptr<struct link_t>& __link,
    const callback_t&                     __callback,
    const quick_callback_t&               __action)
{
    return PROMISE_CALLBACK_R(args, __link, __callback, __action) {
        let result = __callback(args);
        if (result.is_undefined()) __action(__link, { });
        else if (result.class_name() == std::string("impact::promise")) {
            result.get<promise>()
            .done(PROMISE_CALLBACK_R(a, __link) {
                _S_resolve(__link, a);
                return let();
            })
            .fail(PROMISE_CALLBACK_R(a, __link) {
                _S_reject(__link, a);
                return let();
            })
            .progress(PROMISE_CALLBACK_R(a, __link) {
                _S_notify(__link, a);
                return let();
            });
        }
        else __action(__link, { result });
        return let();
    };
}


promise
promise::then(
    callback_t __done,
    callback_t __fail,
    callback_t __progress)
{
    if (m_link == nullptr) throw impact_error("calling then on tail");
    std::shared_ptr<struct link_t> child = std::make_shared<struct link_t>();
    (void)__progress;

    this->done(__done ?
        _M_wrap_callback(child, __done, _S_resolve) :
        PROMISE_CALLBACK_R(args, child) {
            _S_resolve(child, args);
            return let();
        }
    );
    this->fail(
        __fail ?
        _M_wrap_callback(child, __fail, _S_reject) :
        PROMISE_CALLBACK_R(args, child) {
            _S_reject(child, args);
            return let();
        }
    );
    this->progress(
        __progress ?
        _M_wrap_callback(child, __progress, _S_notify) :
        PROMISE_CALLBACK_R(args, child) {
            _S_notify(child, args);
            return let();
        }
    );

    this->m_link->children.push_back(child);
    return promise(child);
}


promise&
promise::_M_done(const std::vector<callback_t>& __callbacks)
{
    for (const auto& callback : __callbacks) {
        if (!callback) continue;
        if (m_link->state_id == state_t::PENDING)
            m_link->done_callbacks.push_back(callback);
        else callback(m_link->final_args);
    }
    return *this;
}


promise&
promise::_M_fail(const std::vector<callback_t>& __callbacks)
{
    for (const auto& callback : __callbacks) {
        if (!callback) continue;
        if (m_link->state_id == state_t::PENDING)
            m_link->fail_callbacks.push_back(callback);
        else callback(m_link->final_args);
    }
    return *this;
}


promise&
promise::_M_progress(const std::vector<callback_t>& __callbacks)
{
    if (m_link->state_id != state_t::PENDING) return *this;
    for (const auto& callback : __callbacks) {
        if (!callback) continue;
        m_link->progress_callbacks.push_back(callback);
    }
    return *this;
}


std::string
promise::state() const
{
    if (m_link) {
        switch (m_link->state_id) {
        case state_t::PENDING:  return "pending";
        case state_t::RESOLVED: return "resolved";
        case state_t::REJECTED: return "rejected";
        }
    }
    
    return "undefined";
}


void
promise::_M_resolve(const std::vector<let>& __args)
{ _S_resolve(m_link, __args); }


void
promise::_M_reject(const std::vector<let>& __args)
{ _S_reject(m_link, __args); }


void
promise::_M_notify(const std::vector<let>& __args)
{ _S_notify(m_link, __args); }


void
promise::_S_resolve(
    const std::shared_ptr<struct link_t>& __link,
    const std::vector<let>&               __args)
{
    if (__link->state_id != state_t::PENDING) return;
    __link->state_id   = state_t::RESOLVED;
    __link->final_args = __args;
    for (const auto& callback : __link->done_callbacks)
        callback(__link->final_args);

    _S_cleanup(__link);
}


void
promise::_S_reject(
    const std::shared_ptr<struct link_t>& __link,
    const std::vector<let>&               __args)
{
    if (__link->state_id != state_t::PENDING) return;
    __link->state_id   = state_t::REJECTED;
    __link->final_args = __args;
    for (const auto& callback : __link->fail_callbacks)
        callback(__link->final_args);

    _S_cleanup(__link);
}


void
promise::_S_notify(
    const std::shared_ptr<struct link_t>& __link,
    const std::vector<let>&               __args)
{
    if (__link->state_id != state_t::PENDING) return;
    for (const auto& callback : __link->progress_callbacks)
        callback(__args);
}


void
promise::_S_cleanup(const std::shared_ptr<struct link_t>& __link)
{
    // these callbacks are no longer needed,
    // so free up unused memory
    __link->done_callbacks.clear();
    __link->fail_callbacks.clear();
    __link->progress_callbacks.clear();
    // also disolve the chain;
    // any externally referenced links will be preserved
    __link->children.clear();
    // the chain is no longer needed because every link
    // will have either resolved or rejected; any new
    // links added will be fired immediately
}


deferred::deferred(std::function<void(deferred&)> __init_cb)
: promise::promise()
{ if (__init_cb) __init_cb(*this); }


promise_proxy_callback::promise_proxy_callback(callback_t __callback)
: m_base(__callback)
{ }


promise_proxy_callback&
promise_proxy_callback::operator=(callback_t __callback)
{
    m_base = __callback;
    return *this;
}


let
promise_proxy_callback::operator() (std::vector<let> __args)
{
    if (m_base) m_base(__args);
    return m_return;
}
