/**
 * Created by TekuConcept on January 1, 2021
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include "async/uv_event_loop.h"

using namespace impact;

#define VERBOSE(x) std::cout << x << std::endl


void
test_sync_start_stop()
{
    VERBOSE("TEST SYNC START/STOP");

    uv_event_loop event_loop;

    std::thread t1([&](){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        VERBOSE("> stopping loop");
        event_loop.stop();
    });

    VERBOSE("> starting loop");
    event_loop.run(); // blocking
    VERBOSE("> loop stopped");

    if (t1.joinable())
        t1.join();

    VERBOSE("END TEST");
    VERBOSE("--------------------");
}


void
test_async_start_stop()
{
    VERBOSE("TEST ASYNC START/STOP");

    {
        uv_event_loop event_loop;
        event_loop.run_async();
        // stop will be automatically called on destruction
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    VERBOSE("END TEST");
    VERBOSE("--------------------");
}


void
test_set_immediate()
{
    VERBOSE("TEST SET/CLEAR IMMEDIATE");

    // call set_immediate synchronously
    {
        uv_event_loop event_loop;
        event_loop.set_immediate([]() {
            VERBOSE("> set_immediate before run");
        });
        // now run it
        event_loop.run_async();
        // NOTE: the callback will be invoked on the next cycle;
        // stop() will also be invoked automatically, nevertheless
        // the callback will always have enough time to run before
        // the event loop stops.
    }

    // call set_immediate followed by clear immediate synchronously
    {
        uv_event_loop event_loop;
        etimer_id_t id = event_loop.set_immediate([]() {
            VERBOSE("> this callback will never be invoked");
        });
        event_loop.clear_immediate(id);
        // now run it
        event_loop.run_async();
        // nothing should happen
    }

    // chain call
    {
        std::promise<void> promise;
        std::future<void> future = promise.get_future();
        uv_event_loop event_loop;
        event_loop.set_immediate([&]() {
            VERBOSE("> first immediate");
            event_loop.set_immediate([&]() {
                VERBOSE("> second immediate");
                promise.set_value();
            });
        });
        // now run it
        event_loop.run_async();
        // wait for callbacks to finish
        future.get();
    }

    // call set_immediate asynchronously
    {
        std::promise<void> promise;
        std::future<void> future = promise.get_future();
        uv_event_loop event_loop;
        // notice how run_async() is called _before_ set_immediate
        event_loop.run_async();
        event_loop.set_immediate([&]() {
            VERBOSE("> set_immediate async");
            promise.set_value();
        });
        // wait for callbacks to finish
        future.get();
    }

    // call set_immediate followed by clear_immediate asynchronously
    {
        uv_event_loop event_loop;
        // notice how run_async() is called _before_ set_immediate
        event_loop.run_async();
        etimer_id_t id = event_loop.set_immediate([&]() {
            VERBOSE("> this callback may or may not be invoked");
        });
        event_loop.clear_immediate(id);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        // nothing should happen
    }

    VERBOSE("END TEST");
    VERBOSE("--------------------");
}


void
test_set_timeout()
{
    VERBOSE("TEST SET/CLEAR TIMEOUT");

    // call set_timeout synchronously
    {
        uv_event_loop event_loop;
        event_loop.set_timeout([]() {
            VERBOSE("> set_timeout before run");
        }, 250/*ms*/);
        // now run it
        event_loop.run_async();
        // ...and wait
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // call set_timeout followed by clear timeout synchronously
    {
        uv_event_loop event_loop;
        etimer_id_t id = event_loop.set_timeout([]() {
            VERBOSE("> this callback will never be invoked");
        }, 250/*ms*/);
        event_loop.clear_timeout(id);
        // now run it
        event_loop.run_async();
        // ...and wait
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        // nothing should happen
    }

    // chain call
    {
        std::promise<void> promise;
        std::future<void> future = promise.get_future();
        uv_event_loop event_loop;
        event_loop.set_timeout([&]() {
            VERBOSE("> first timeout");
            event_loop.set_timeout([&]() {
                VERBOSE("> second timeout");
                promise.set_value();
            }, 250/*ms*/);
        }, 250/*ms*/);
        // now run it
        event_loop.run_async();
        // wait for callbacks to finish
        future.get();
    }

    // call set_timeout asynchronously
    {
        std::promise<void> promise;
        std::future<void> future = promise.get_future();
        uv_event_loop event_loop;
        // notice how run_async() is called _before_ set_timeout
        event_loop.run_async();
        event_loop.set_timeout([&]() {
            VERBOSE("> set_timeout async");
            promise.set_value();
        }, 250/*ms*/);
        // wait for callbacks to finish
        future.get();
    }

    // call set_timeout followed by clear_timeout asynchronously
    {
        uv_event_loop event_loop;
        // notice how run_async() is called _before_ set_timeout
        event_loop.run_async();
        etimer_id_t id = event_loop.set_timeout([&]() {
            VERBOSE("> this callback may or may not be invoked");
        }, 500/*ms*/);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        event_loop.clear_timeout(id);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        // nothing should happen
    }

    VERBOSE("END TEST");
    VERBOSE("--------------------");
}


void
test_set_interval()
{
    VERBOSE("TEST SET/CLEAR INTERVAL");

    // call set_interval synchronously
    {
        uv_event_loop event_loop;
        event_loop.set_interval([]() {
            VERBOSE("> set_interval before run");
        }, 250/*ms*/);
        // now run it
        event_loop.run_async();
        // ...and wait
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        // should invoke the callback 3-4 times
    }

    // call set_interval followed by clear interval synchronously
    {
        uv_event_loop event_loop;
        etimer_id_t id = event_loop.set_interval([&]() {
            VERBOSE("> this callback is invoked once");
            event_loop.clear_interval(id);
        }, 250/*ms*/);
        // now run it
        event_loop.run_async();
        // ...and wait
        std::this_thread::sleep_for(std::chrono::milliseconds(750));
    }

    // call set_interval asynchronously
    {
        uv_event_loop event_loop;
        // notice how run_async() is called _before_ set_interval
        event_loop.run_async();
        event_loop.set_interval([]() {
            VERBOSE("> set_interval async");
        }, 250/*ms*/);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        // should invoke the callback about 3 times
    }

    // call set_interval followed by clear_interval asynchronously
    {
        uv_event_loop event_loop;
        // notice how run_async() is called _before_ set_interval
        event_loop.run_async();
        etimer_id_t id = event_loop.set_interval([&]() {
            VERBOSE("> this interval will be stopped asynchronously");
        }, 200/*ms*/);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        event_loop.clear_interval(id);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        // should invoke the callback 2-3 times
    }

    VERBOSE("END TEST");
    VERBOSE("--------------------");
}


void
test_invoke()
{
    VERBOSE("TEST ASYNC INVOKE");

    uv_event_loop event_loop;
    event_loop.run_async();

    {
        std::promise<void> p;
        std::future<void> f = p.get_future();
        event_loop.invoke([&]() {
            VERBOSE("> non-blocking invoke");
            p.set_value();
        });
        f.get();
    }

    {
        event_loop.invoke([]() {
            VERBOSE("> blocking invoke");
        }, /*blocking=*/true);
    }

    VERBOSE("END TEST");
    VERBOSE("--------------------");
}


int main() {
    VERBOSE("- BEGIN uv_event_loop TEST -");
    VERBOSE("--------------------");

    test_sync_start_stop();
    test_async_start_stop();
    test_set_immediate();
    test_set_timeout();
    test_set_interval();
    test_invoke();

    VERBOSE("- END OF LINE -");
    return 0;    
}
