/**
 * Created by TekuConcept on January 19, 2021
 */

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <future>
#include "async/uv_event_loop.h"

using namespace impact;

#define VERBOSE(x) std::cout << x << std::endl

int main() {
    VERBOSE("- BEGIN tcp_server_socket TEST -");

    std::shared_ptr<event_loop_interface> event_loop =
        std::shared_ptr<event_loop_interface>(new uv_event_loop());

    std::thread t([&](){
        std::this_thread::sleep_for(std::chrono::seconds(2));
        tcp_server_pt server = event_loop->create_tcp_server();

        std::promise<void> p1, p2;
        std::future<void> f1 = p1.get_future();
        std::future<void> f2 = p2.get_future();

        server->listen(7000, EVENT_LISTENER(, &) {
            std::cout << "server is listening" << std::endl;
            p1.set_value();
        });
        f1.get(); // wait until server is listening

        std::this_thread::sleep_for(std::chrono::seconds(15));
        std::cout << "thread is closing" << std::endl;

        server->close(EVENT_LISTENER(, &) {
            std::cout << "closing server" << std::endl;
            p2.set_value();
        });
        // when calling close manually on a thread other than the
        // event loop, dtor cannot be called until close finished
        f2.get();

        std::cout << "stopping event loop..." << std::endl;
        event_loop->stop();
        std::cout << "stop request sent" << std::endl;
    });

    event_loop->run();

    std::cout << "ready to join twin thread..." << std::endl;
    if (t.joinable()) t.join();
    std::cout << "thread joined" << std::endl;

    VERBOSE("- END OF LINE -");
    return 0;
}
