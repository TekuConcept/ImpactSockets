/**
 * Created by TekuConcept on February 24, 2020
 */

#include <thread>
#include <chrono>
#include <iostream>
#include "async/event_loop.h"

using namespace impact;

int main() {
    auto loop = event_loop(2);
    for (size_t i = 0; i < 3; i++) {
        loop.add_idle_event([=](size_t tip, void*) {
            // An idle event, or in other words, a callback
            // invoked by one of several threads that have
            // nothing else better to do.
            std::cout << tip << "I" << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }, (void*)i);
    }

    for (size_t i = 0; i < 10; i++) {
        std::cout << "-" << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (i == 3) {
            loop.push_event([](size_t tip) {
                // A normal event, or in other words, a callback
                // invoked by one of several available threads.
                std::cout << tip << "E" << std::endl;
            });
        }
        else if (i == 5) {
            for (size_t i = 0; i < 3; i++)
                loop.remove_idle_event((void*)i);
        }
    }

    return 0;
}
