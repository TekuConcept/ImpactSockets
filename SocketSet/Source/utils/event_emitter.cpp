/**
 * Created by TekuConcept on January 18, 2021
 */

#include <iostream>
#include <set>
#include <algorithm>
#include "utils/event_emitter.h"

using namespace impact;


size_t event_emitter::default_max_listeners = 10;


event_emitter::event_emitter()
: m_context(std::make_shared<struct context_t>())
{ m_context->max_listeners = default_max_listeners; }


std::vector<std::string>
event_emitter::event_names() const noexcept
{
    std::set<std::string> names;
    for (const auto& token : m_context->listeners)
        names.insert(token.first);
    return std::vector<std::string>(names.begin(), names.end());
}


size_t
event_emitter::listener_count(const std::string& __name) const noexcept
{
    const auto& token = m_context->listeners.find(__name);
    if (token != m_context->listeners.end())
        return token->second.size();
    else return 0;
}


const std::vector<event_emitter::callback_info>&
event_emitter::listeners(const std::string& __name) const noexcept
{
    const auto& token = m_context->listeners.find(__name);
    if (token == m_context->listeners.end())
        return m_context->empty_list;
    else return token->second;
}


void
event_emitter::remove_listener(
    const std::string& __name,
    const callback_t&  __listener)
{
    const auto& token = m_context->listeners.find(__name);
    if (token != m_context->listeners.end()) {
        const auto& info = std::find_if(
            token->second.begin(),
            token->second.end(),
        [&](const struct callback_info& next) -> bool {
            return next.id == (size_t)&__listener;
        });
        if (info != token->second.end())
            token->second.erase(info);
        // erase event name if no more listeners
        if (token->second.size() == 0)
            m_context->listeners.erase(token);
    }
}


void
event_emitter::remove_all_listeners(const std::string& __name) noexcept
{
    const auto& token = m_context->listeners.find(__name);
    if (token != m_context->listeners.end())
        m_context->listeners.erase(token);
}


void
event_emitter::_M_emit(
    const std::string&      __name,
    const std::vector<let>& __args)
{
    const auto& token = m_context->listeners.find(__name);
    if (token == m_context->listeners.end()) return;

    auto& listeners = token->second;
    for (const auto& info : listeners)
        info.cb(__args);

    for (size_t i = 0; i < listeners.size(); i++) {
        if (listeners[i].once) {
            listeners.erase(listeners.begin() + i);
            i--;
        }
    }

    // erase event name if no more listeners
    if (listeners.size() == 0)
        m_context->listeners.erase(token);
}


void
event_emitter::_M_add_listener(
    const std::string& __name,
    const callback_t&  __listener,
    bool               __once,
    bool               __prepend)
{
    if (__name.size() == 0) return;
    if (!__listener) return;

    auto& list = m_context->listeners[__name];
    if (list.size() == m_context->max_listeners) return;

    callback_info info;
    info.id   = (size_t)&__listener;
    info.cb   = __listener;
    info.once = __once;

    if (!__prepend)
        list.push_back(info);
    else list.insert(list.begin(), info);
}
