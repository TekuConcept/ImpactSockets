/**
 * Created by TekuConcept on December 31, 2020
 */

#ifndef IMPACT_ABSTRACT_EVENT_LOOP_H
#define IMPACT_ABSTRACT_EVENT_LOOP_H

#include <cstdint>
#include <functional>


namespace impact {

    // FIXME: uintptr_t type is not guaranteed to exist
    typedef std::uintptr_t etimer_id_t;
    typedef unsigned long long int etimer_time_t;
    typedef std::function<void()> etimer_callback_t;

    class abstract_event_loop {
    public:
        virtual ~abstract_event_loop() = default;

        virtual void run() = 0;
        virtual void run_async() = 0;
        virtual void stop() = 0;

        virtual etimer_id_t set_timeout(etimer_callback_t cb, etimer_time_t ms) = 0;
        virtual etimer_id_t set_interval(etimer_callback_t cb, etimer_time_t ms) = 0;
        virtual etimer_id_t set_immediate(etimer_callback_t cb) = 0;
        virtual void clear_timeout(etimer_id_t id) = 0;
        virtual void clear_interval(etimer_id_t id) = 0;
        virtual void clear_immediate(etimer_id_t id) = 0;

        // async IO (send, receive)
    };

} /* namespace impact */

#endif
