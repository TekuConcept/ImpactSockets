/**
 * Created by TekuConcept on March 12, 2021
 */

#include <gtest/gtest.h>
#include "sockets/gnutls_secure_datagram.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <memory>
#include "async/uv_event_loop.h"
#include "sockets/gnutls_secure_datagram.h"
#include "utils/impact_error.h"

#define VERBOSE(x) std::cout << x << std::endl
#define AS_STRING(arg) (arg).get<std::string>()

using namespace impact;

std::shared_ptr<event_loop_interface> s_event_loop;
std::string test_message;
secure_datagram_t datagram;


/**
 * openssl genrsa -out private-key.pem 1024
 * openssl req -new -key private-key.pem -out csr.pem
 * openssl x509 -req -in csr.pem -signkey private-key.pem -out public-cert.pem
 * openssl s_client -connect 127.0.0.1:8000
 * 
 * mkdir -p demoCA &\
 * touch demoCA/index.txt &\
 * touch demoCA/index.txt.attr &\
 * echo 1000 > demoCA/crlnumber
 * openssl rand -out ~/.rnd -hex 256
 * openssl ca -keyfile private-key.pem -cert public-cert.pem -gencrl -out crl.pem
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


void create_datagram_socket(unsigned short port) {
    auto base_datagram = s_event_loop->create_udp_socket();
    datagram = secure_datagram_t(
        new gnutls_secure_datagram(base_datagram));
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
    VERBOSE("-- BEGIN TLS UDP SOCKETS TEST --");

    /**
     * The UDP socket will try to send a message to itself.
     */

    test_message = "hello world";
    unsigned short port = 41234;
    udp_address_t loopback = {
        .port    = port,
        .family  = address_family::INET,
        .address = "127.0.0.1"
    };

    s_event_loop = std::shared_ptr<event_loop_interface>(new uv_event_loop());
    s_event_loop->run_async();

    create_datagram_socket(port);

    // create a new connection with the loopback address
    datagram->create(loopback);

    { // set certificate credentials
        auto key  = read_file("private-key.pem");
        auto cert = read_file("public-cert.pem");
        datagram->set_x509_client_credentials(loopback, key, cert);
        datagram->set_x509_credentials(key, cert);
    }

    // wait for loopback connection to become ready
    datagram->on("ready", EVENT_LISTENER(args, &) {
        udp_address_t address = args[0].get<udp_address_t>();
        VERBOSE("EVENT: [ready] " << address.address << ":" << address.port);
        datagram->send(test_message, address.port, address.address);
    });

    // begin the loopback connection
    datagram->begin(loopback);

    /**
     * Give the program some time to run
     * (the event loop will automatically stop when main returns)
     */

    std::this_thread::sleep_for(std::chrono::seconds(3));

    /* gracefully close the socket */
    datagram->close();

    VERBOSE("-- END OF LINE --");
    return 0;
}
