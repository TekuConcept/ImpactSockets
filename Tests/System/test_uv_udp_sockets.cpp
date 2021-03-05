/**
 * Created by TekuConcept on March 5, 2021
 */

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <future>
#include "async/uv_event_loop.h"

using namespace impact;

#define VERBOSE(x) std::cout << x << std::endl
#define AS_STRING(arg) (arg).get<std::string>()

std::string test_message;
std::shared_ptr<event_loop_interface> s_event_loop;
udp_socket_t datagram;


void create_datagram_socket(unsigned short port) {
    datagram = s_event_loop->create_udp_socket();
    datagram->on("error", EVENT_LISTENER(args, &) {
        VERBOSE("EVENT: [error] " << AS_STRING(args[0]));
        datagram->close();
    });
    datagram->on("message", EVENT_LISTENER(args, &) {
        auto address = args[1].get<udp_address_t>();
        auto message = AS_STRING(args[0]);
        if (message.size() > 0) {
            VERBOSE("EVENT: [data] " << message << " from "
                << address.address << ":" << address.port);
            if (message == test_message) VERBOSE("ECHO TEST: PASS");
            else VERBOSE("ECHO TEST: FAIL\nUnexpected message: " << message);
        }
        // else ignore empty datagrams
    });
    datagram->on("listening", EVENT_LISTENER(, &) {
        auto address = datagram->address();
        VERBOSE("EVENT: socket listening on "
            << address.address << ":" << address.port);
    });
    datagram->bind(port);
}


int main() {
    VERBOSE("- BEGIN tcp_server_socket TEST -");

    /**
     * The UDP socket will try to send a message to itself.
     */

    test_message = "hello world";
    unsigned short port = 41234;
    s_event_loop = std::shared_ptr<event_loop_interface>(new uv_event_loop());
    s_event_loop->run_async();

    create_datagram_socket(port);

    datagram->send(test_message, port);

    /**
     * Give the program some time to run
     * (the event loop will automatically stop when main returns)
     */

    std::this_thread::sleep_for(std::chrono::seconds(2));

    /* gracefully close the socket */
    datagram->close();

    VERBOSE("- END OF LINE -");
    return 0;
}
