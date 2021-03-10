/**
 * Created by TekuConcept on January 19, 2021
 */

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <future>
#include "async/uv_event_loop.h"
#include "utils/impact_error.h"

using namespace impact;

#define VERBOSE(x) std::cout << x << std::endl
#define AS_STRING(arg) (arg).get<std::string>()

std::string test_message;
std::shared_ptr<event_loop_interface> s_event_loop;
tcp_server_t server;
tcp_client_t server_connection;
tcp_client_t client_connection;


void create_connection() {
    client_connection = s_event_loop->create_tcp_client();
    client_connection->on("data", EVENT_LISTENER(data, &) {
        auto response = AS_STRING(data[0]);
        VERBOSE("EVENT: [data] " << response);
        if (response == test_message) {
            VERBOSE("ECHO TEST: PASS");
            client_connection->destroy();
        }
        else VERBOSE("ECHO TEST: FAIL\nUnexpected response: " << response);
    });
    client_connection->on("connect", EVENT_LISTENER(, &) {
        VERBOSE("EVENT: client connected to server");
        client_connection->write(test_message);
    });
    client_connection->on("error", EVENT_LISTENER(error, &) {
        VERBOSE("EVENT: client error: " << AS_STRING(error[0]));
    });
    client_connection->connect(7000);
}


void create_server() {
    server = s_event_loop->create_tcp_server();
    server->on("connection", EVENT_LISTENER(args, &) {
        server_connection = args[0].get<tcp_client_t>();
        server_connection->on("data", EVENT_LISTENER(data, &) {
            std::string message = AS_STRING(data[0]);
            VERBOSE("EVENT: [server-data] " << message);
            server_connection->write(message);
        });
        server_connection->on("close", EVENT_LISTENER(, &) {
            VERBOSE("EVENT: server-connection closed");
            server->close();
        });
    });
    server->on("listening", EVENT_LISTENER() {
        VERBOSE("EVENT: server listening on port 7000");
        create_connection();
    });
    server->listen(7000);
}


int main() {
    VERBOSE("- BEGIN tcp_server_socket TEST -");

    /**
     * The client will send a message and the server will echo
     * the message back to the client. Once the client receives
     * the response, it will close the connection. When the
     * server connection closes, the server will shutdown as well.
     */

    test_message = "hello world";
    s_event_loop = std::shared_ptr<event_loop_interface>(new uv_event_loop());
    s_event_loop->run_async();

    create_server();

    /**
     * Give the program some time to run
     * (the event loop will automatically stop when main returns)
     */

    std::this_thread::sleep_for(std::chrono::seconds(2));

    s_event_loop->stop();

    VERBOSE("- END OF LINE -");
    return 0;
}

/**
 * Note: on('error') events will not be emitted when an error
 *       occurs after the object's destructor has been called.
 */
