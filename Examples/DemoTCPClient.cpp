#include <iostream>
#include <thread>
#include <chrono>
#include <string>

#include <basic_socket>
#include <socketstream>

using namespace impact;

#define VERBOSE(x) std::cout << x << std::endl

int main() {
    VERBOSE("- BEGIN TCP DEMO -");

    try {
        // run on main thread
        basic_socket socket = make_tcp_socket();
        socket.connect(25565);
        socketstream stream(socket);

        VERBOSE("- CLIENT CONNECTED");

        stream << "Hello From Client" << std::endl;

        std::string message;
        if(!std::getline(stream, message)) {
            VERBOSE("- Could not get server message.");
            VERBOSE("- END OF LINE -");
            return 1;
        }
        VERBOSE("- Message: " << message);

        stream << "I got your message!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        stream << "...sorry I was late" << std::endl;

        VERBOSE("- CLIENT SHUTTING DOWN");

        socket.close();
    }
    catch (std::exception e) {
        VERBOSE("- Error: " << e.what());
    }

    VERBOSE("- END OF TCP DEMO -");
    return 0;
}
