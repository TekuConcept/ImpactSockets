#include <iostream>
#include <cassert>

#include <basic_socket>
#include <impact_error>

#define VERBOSE(x) std::cout << x << std::endl

using namespace impact;

void
test_default_constructor()
{
    VERBOSE("\nTest Default Constructor");
    VERBOSE("[1]");
    basic_socket socket;
    assert(socket.use_count() == 1);
    VERBOSE("[2]");
    basic_socket soc2 = socket;
    assert(socket.use_count() == 2);
    VERBOSE("[3]");
    basic_socket soc3 = std::move(socket);
    assert(socket.use_count() == 0);
    socket = std::move(soc3); // move back
    VERBOSE("[4]");
    basic_socket soc4(socket);
    assert(socket.use_count() == 3);
    VERBOSE("[5]");
    basic_socket soc5(std::move(socket));
    assert(socket.use_count() == 0);
    VERBOSE("Done!");
}


void
test_create_constructor()
{
    VERBOSE("\nTest Create Constructor");
    VERBOSE("[1]");
    basic_socket socket;
    assert(socket.use_count() == 1);
    VERBOSE("[2]");
    socket = make_tcp_socket();
    assert(socket.use_count() == 1);
    {
        VERBOSE("[3]");
        basic_socket soc2 = socket;
        assert(socket.use_count() == 2);
    }
    {
        VERBOSE("[4]");
        basic_socket soc3(socket);
        assert(socket.use_count() == 2);
    }
    {
        VERBOSE("[5]");
        basic_socket soc4 = std::move(socket);
        assert(socket.use_count() == 0);
        assert(soc4.use_count() == 1);
    }
    VERBOSE("Done!");
}


void
test_close()
{
    VERBOSE("\nTest Close");
    basic_socket socket;

    VERBOSE("[1]");
    try {
        socket.close();
        assert(false);
    } catch (...) { }

    socket = make_tcp_socket();

    VERBOSE("[2]");
    try {
        socket.close();
    } catch (...) { assert(false); }
    VERBOSE("Done!");
}


void
test_sigpipe()
{
    VERBOSE("\nTest Pipe Error Signal");
    VERBOSE("[1]");
    basic_socket socket;
    try { socket = make_tcp_socket(); } catch (...) { assert(false); }
  
    try {
        const char* buffer = NULL;
        socket.send(buffer, 0, message_flags::NONE);
    }
    catch (impact_error& e) {
        VERBOSE("IO Error: " << e.message());
        assert(true);
    }
    catch (...) {
        VERBOSE("Unknown error");
        assert(false);
    }
  
    try { socket.close(); } catch (...) { assert(false); }
    VERBOSE("Done!");
}


int main() {
    VERBOSE("- BEGIN -");

    test_default_constructor();
    test_create_constructor();
    test_close();
    test_sigpipe();

    VERBOSE("- END OF LINE -");
    return 0;
}
