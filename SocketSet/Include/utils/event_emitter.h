/**
 * Created by TekuConcept on January 18, 2021
 */

#ifndef IMPACT_EVENT_EMITTER_H
#define IMPACT_EVENT_EMITTER_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include "utils/let.h"

namespace impact {

    #define EVENT_LISTENER(name, ...) \
        (event_emitter::callback_t)[__VA_ARGS__](const std::vector<let>& name) -> void

    class event_emitter {
    public:
        typedef std::function<void(const std::vector<let>&)> callback_t;
        struct callback_info {
            // a pointer is used to identify the same callback
            // callback copies cannot be identified otherwise
            size_t id;
            // a copy is saved in case the external source
            // goes out of scope eg anonymous lambdas
            callback_t cb;
            // should this callback be removed after it is invoked
            bool once;
            // initialize defaults
            callback_info();
        };

        event_emitter();
        event_emitter(const event_emitter* forward);
        virtual ~event_emitter() = default;

        inline void forward(const event_emitter* forward)
        { if (forward) this->m_context = forward->m_context; }

        template <class ... ARGS>
        void emit(const std::string& name, ARGS ... args)
        { _M_emit(name, { args... }); };

        std::vector<std::string> event_names() const noexcept;
        inline size_t get_max_listeners() const noexcept
        { return m_context->max_listeners; }
        inline void set_max_listeners(size_t value) noexcept
        { m_context->max_listeners = value; }
        size_t listener_count(const std::string& name) const noexcept;
        const std::vector<callback_info>& listeners(const std::string& name) const noexcept;

        inline void add_listener(const std::string& name, const callback_t& listener)
        { _M_add_listener(name, listener, false, false); }
        inline void once(std::string name, const callback_t& listener)
        { _M_add_listener(name, listener, true, false); }
        inline void prepend_listener(std::string name, const callback_t& listener)
        { _M_add_listener(name, listener, false, true); }
        inline void prepend_once_listener(std::string name, const callback_t& listener)
        { _M_add_listener(name, listener, true, true); }
        void remove_listener(const std::string& name, const callback_t& listener);
        void remove_all_listeners(const std::string& name) noexcept;

        inline void off(std::string name, const callback_t& listener)
        { remove_listener(name, listener); }
        inline void on(std::string name, const callback_t& listener)
        { add_listener(name, listener); }

        static size_t default_max_listeners;

    private:
        struct context_t {
            std::vector<callback_info> empty_list;
            std::map<std::string,std::vector<callback_info>> listeners;
            size_t max_listeners;
            context_t();
        };

        std::shared_ptr<struct context_t> m_context;

        void _M_emit(const std::string&, const std::vector<let>&);
        void _M_add_listener(const std::string&, const callback_t&, bool, bool);
    };

} /* namespace impact */


#endif /* IMPACT_EVENT_EMITTER_H */
