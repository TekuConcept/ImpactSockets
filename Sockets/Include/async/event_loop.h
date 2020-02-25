/**
 * Created by TekuConcept on February 24, 2020
 */

#ifndef IMPACT_EVENT_LOOP_H
#define IMPACT_EVENT_LOOP_H

#include <memory>
#include <functional>

namespace impact {

    class event_loop_base {
    public:
        virtual ~event_loop_base() = default;
        virtual bool add_idle_event(
            std::function<void(size_t, void*)> function,
            void* function_id) = 0;
        virtual bool remove_idle_event(void* function_id) = 0;
        virtual void push_event(std::function<void(size_t)> function) = 0;
    };


    class event_loop : public event_loop_base {
    public:
        event_loop(size_t poolsize = 1);
        event_loop(std::shared_ptr<event_loop_base> base);
        ~event_loop() = default;

        inline bool add_idle_event(
            std::function<void(size_t, void*)> function,
            void* function_id)
        override { return m_base_->add_idle_event(function, function_id); }
        inline bool remove_idle_event(void* function_id)
        override { return m_base_->remove_idle_event(function_id); }
        inline void push_event(std::function<void(size_t)> function)
        override { m_base_->push_event(function); }

    private:
        std::shared_ptr<event_loop_base> m_base_;
    };

}

#endif
