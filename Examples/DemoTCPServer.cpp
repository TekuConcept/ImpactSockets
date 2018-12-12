#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include <basic_socket>
#include <impact_error>
#include <socketstream>

using namespace impact;

#define VERBOSE(x) std::cout << x << std::endl

int main() {
    VERBOSE("- BEGIN TCP DEMO -");

    try {
        basic_socket server = make_tcp_socket();
        server.bind(25565);
        server.listen();

        VERBOSE("+ SERVER STARTED");

        basic_socket client = server.accept();
        socketstream stream(client);
        stream.set_timeout(2500); // 2.5s

        VERBOSE("+ FOUND NEW CLIENT");

        std::string message;
        std::getline(stream, message);
        VERBOSE("+ Message: " << message);

        stream << "Hello From Server" << std::endl;

        std::getline(stream, message);
        VERBOSE("+ Message: " << message);

        // attempt to wait for client but timeout
        // because nothing arrives
        std::string late_message = "- NO MESSAGE -";
        std::getline(stream, late_message);
        VERBOSE("+ Fail: " << stream.fail());
        VERBOSE("+ Bad:  " << stream.bad());
        VERBOSE("+ Message: " << late_message);

        VERBOSE("+ SERVER SHUTTING DOWN");

        try { client.close(); }
        catch (...) { VERBOSE("+ Client error on close"); }
        try { server.close(); }
        catch (impact_error e) {
            VERBOSE("+ Server close error " << e.what());
        }
    }
    catch (impact_error e) { VERBOSE("+ Error: " << e.what()); }
    catch (...) { VERBOSE("Unknown internal error"); }

    VERBOSE("- END OF TCP DEMO -");
    return 0;
}
