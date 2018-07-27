/**
 * Created by TekuConcept on July 26, 2017
 */

#include <csignal>
#include <iostream>
#include <future>
#include <thread>
#include <chrono>

#include "sockets/environment.h"
#include "sockets/async_pipeline.h"
#include "sockets/impact_error.h"
#include "sockets/generic.h"

// valgrind
// --leak-check=full
// --show-leak-kinds=all
// --track-origins=yes
// --log-file=run.log
// ./program

// #define VERBOSE(x) std::cout << x << std::endl
#define VERBOSE(x)
#define TEST(x) std::cout << x << std::endl;

using async_pipeline   = impact::internal::async_pipeline;
using async_object_ptr = impact::internal::async_object_ptr;
using async_functor    = impact::internal::async_functor;
using poll_handle      = impact::poll_handle;
using poll_flags       = impact::poll_flags;
using socket_error     = impact::socket_error;

void signal_handler(int signo) {
    switch(signo) {
    case SIGABRT: TEST("Signal: Abort"); break;
    case SIGFPE:  TEST("Signal: Floating-Point Error"); break;
    case SIGILL:  TEST("Signal: Illegal"); break;
    case SIGINT:  TEST("Signal: Interrupt"); break;
    case SIGSEGV: TEST("Signal: Segmentation Fault"); break;
    case SIGTERM: TEST("Signal: Terminate"); break;
    default:      TEST("Other: " << signo); break;
    }
}

void register_signals() {
    if (std::signal(SIGABRT, signal_handler) == SIG_ERR) { TEST("SigFail: SIGABRT"); }
    if (std::signal(SIGFPE,  signal_handler) == SIG_ERR) { TEST("SigFail: SIGFPE"); }
    if (std::signal(SIGILL,  signal_handler) == SIG_ERR) { TEST("SigFail: SIGILL"); }
    if (std::signal(SIGINT , signal_handler) == SIG_ERR) { TEST("SigFail: SIGINT"); }
    if (std::signal(SIGSEGV, signal_handler) == SIG_ERR) { TEST("SigFail: SIGSEGV"); }
    if (std::signal(SIGTERM, signal_handler) == SIG_ERR) { TEST("SigFail: SIGTERM"); }
}

int main() {
    register_signals();
    VERBOSE("- BEGIN -");
    
    VERBOSE("> 1. Creating pipeline instance");
    auto& pipeline = async_pipeline::instance();
    
    VERBOSE("> 2. Starting socket server");
    impact::basic_socket server = impact::make_tcp_socket();
    server.bind(0);
    server.listen();
    VERBOSE("> 3. Server running");
    
    VERBOSE("> 4. Creating client");
    impact::basic_socket client = impact::make_tcp_socket();
    
    VERBOSE("> 5. Connecting...");
    auto client_future = std::async(std::launch::async,[&](){
        client.connect(server.local_port());
    });
    UNUSED(client_future);
    impact::basic_socket server_peer = server.accept();
    VERBOSE("> 6. Done!");
    
    VERBOSE("> 7. Setting up pipeline");
    auto id = client.get();
    async_object_ptr client_func = std::make_shared<async_functor>(
    [&](poll_handle* handle, socket_error error) {
        if (error != socket_error::SUCCESS) {
            VERBOSE(">> [Client] error: " << impact::internal::error_message());
        }
        else if (handle->return_events & (int)poll_flags::IN) {
            std::string buffer(100, '\0');
            try {
                auto status = client.recv(&buffer[0], buffer.size());
                if (status) {
                    if ((unsigned int)status < buffer.size())
                        buffer[buffer.size()-1] = '\0';
                    VERBOSE(">> [Client] " << buffer);
                }
                else { VERBOSE(">> [Client] EOF"); /* remove from queue */ }
            } catch (impact::impact_error e) {
                VERBOSE(">> [Client] " << e.message());
            }
        }
        else if (
            handle->return_events & (int)poll_flags::HANGUP ||
            handle->return_events & (int)poll_flags::ERROR  ||
            handle->return_events & (int)poll_flags::INVALID
        ) { VERBOSE(">> [Client] error detected"); /* remove from queue */ }
    });
    pipeline.add_object(id, client_func);
    VERBOSE("> 8. Done!");
    
    VERBOSE("-: Beging async test :-");
    
    server_peer.send("Hello World!", 12);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // removal might not happen before end of main
    pipeline.remove_object(client.get());
    
    VERBOSE("-:  End async test   :-");
    
    VERBOSE("> 9. Shutting down");
    try { client.close();      } catch (...) { VERBOSE("ECLOSE 1"); }
    try { server_peer.close(); } catch (...) { VERBOSE("ECLOSE 2"); }
    try { server.close();      } catch (...) { VERBOSE("ECLOSE 3"); }
    
    VERBOSE("- END OF LINE -");
    TEST('.');
    return 0;
}