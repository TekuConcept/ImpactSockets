/**
 * Created by TekuConcept on March 5, 2021
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <memory>
#include "async/uv_event_loop.h"
#include "sockets/gnutls_secure_client.h"
#include "utils/impact_error.h"

#define VERBOSE(x) std::cout << x << std::endl
#define AS_STRING(arg) (arg).get<std::string>()

using namespace impact;

std::shared_ptr<event_loop_interface> s_event_loop;


/**
 * openssl genrsa -out private-key.pem 1024
 * openssl req -new -key private-key.pem -out csr.pem
 * openssl x509 -req -in csr.pem -signkey private-key.pem -out public-cert.pem
 * openssl s_client -connect 127.0.0.1:8000
 */


std::string
read_file(std::string file_name)
{
    std::ostringstream buffer;
    std::ifstream file(file_name);
    if (!file.is_open()) {
        std::cerr << "cannot open file " << file_name << std::endl;
        exit(1);
    }
    buffer << file.rdbuf();
    return buffer.str();
}


int main() {
    VERBOSE("-- BEGIN TLS TCP SOCKETS TEST --");

    //
    // read in credentials
    //
    auto key  = read_file("private-key.pem");
    auto cert = read_file("public-cert.pem");

    //
    // setup async event loop
    //
    s_event_loop = std::shared_ptr<event_loop_interface>(new uv_event_loop());
    s_event_loop->run_async();

    //
    // create a new client
    //
    tcp_client_t client = s_event_loop->create_tcp_client();
    secure_client_t secure_client =
        secure_client_t(new gnutls_secure_client(client));

    //
    // setup event callbacks
    //
    size_t state = 0;
    secure_client->on("error", EVENT_LISTENER(args, &) {
        VERBOSE("[EVENT] error: " << AS_STRING(args[0]));
        secure_client->destroy();
    });
    secure_client->on("data", EVENT_LISTENER(args, &) {
        VERBOSE("[EVENT] data: " << AS_STRING(args[0]));
        if (state < 1) secure_client->write("echo client");
        else if (state < 2) secure_client->end();
        state++;
    });
    secure_client->on("connect", EVENT_LISTENER() {
        VERBOSE("[EVENT] connect");
    });
    secure_client->on("ready", EVENT_LISTENER() {
        VERBOSE("[EVENT] ready");
    });
    secure_client->on("close", EVENT_LISTENER() {
        VERBOSE("[EVENT] close");
    });

    //
    // start the connection
    //
    secure_client->set_x509_credentials(key, cert);
    secure_client->connect(8000);

    //
    // wait a while before shutting down
    //
    std::this_thread::sleep_for(std::chrono::seconds(2));
    s_event_loop->stop();

    VERBOSE("-- END OF LINE --");
    return 1;
}
