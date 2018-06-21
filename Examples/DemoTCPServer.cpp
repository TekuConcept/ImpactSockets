#include <TcpServer.h>
#include <iostream>
#include <string>
#include <stdexcept>
// #include "StaticCommand.h"

// void callback(Object&,EventArgs) {
//     std::cerr << "Read Timed Out [Intentional]" << std::endl;
// }

#define VERBOSE(x) std::cout << x << std::endl

int main() {
    Impact::TcpServer server;

    try { server.open(25565); }
    catch (std::runtime_error e) { VERBOSE(e.what()); return -1; }
    
    VERBOSE("- SERVER STARTED -");
    
    Impact::TcpSocket connection;

    try {
        server.accept(connection);
        connection.setTimeout(2500); // 2.5 seconds
        // connection->onTimeout += StaticCommandPtr(
        //     EventArgs,
        //     callback
        // );
    }
    catch (std::runtime_error e) { VERBOSE(e.what()); return 1; }
    
    VERBOSE("Found new Client");
    
    std::string msg;
    std::getline(connection, msg);
    std::cout << "msg: " << msg << std::endl;

    connection << "Hello From Server" << std::endl;
    
    std::string done;
    std::getline(connection, done);
    VERBOSE("done: " << done);
    
    // attempt to wait for client but timeout
    // because nothing arrives
    std::string latemsg = "- NO MESSAGE -";
    std::getline(connection, latemsg);
    VERBOSE("Fail: " << connection.fail());
    VERBOSE("Bad:  " << connection.bad());
    VERBOSE(latemsg);

    // shouldn't throw except for a system io error
    // or if close was called before open, nevertheless
    // try-catch for demo purposes
    try { connection.close(); } catch (...) {}
    try { server.close(); } catch (...) {}

    VERBOSE("- END OF LINE -");
    
    return 0;
}
