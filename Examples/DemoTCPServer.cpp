// #include <TcpServer.h>
// #include <iostream>
// #include <string>
// #include <stdexcept>
// // #include "StaticCommand.h"
//
// // void callback(Object&,EventArgs) {
// //     std::cerr << "Read Timed Out [Intentional]" << std::endl;
// // }
//
// #define VERBOSE(x) std::cout << x << std::endl
//
// int main() {
//     Impact::TcpServer server;
//
//     try { server.open(25565); }
//     catch (std::runtime_error e) { VERBOSE(e.what()); return -1; }
//
//     VERBOSE("- SERVER STARTED -");
//
//     Impact::TcpSocket connection;
//     try {
//         server.accept(connection);
//         connection.setTimeout(2500); // 2.5 seconds
//         // connection->onTimeout += StaticCommandPtr(
//         //     EventArgs,
//         //     callback
//         // );
//     }
//     catch (std::runtime_error e) { VERBOSE(e.what()); return 1; }
//
//     VERBOSE("Found new Client");
//
//     std::string msg;
//     std::getline(connection, msg);
//     std::cout << "msg: " << msg << std::endl;
//
//     connection << "Hello From Server" << std::endl;
//
//     std::string done;
//     std::getline(connection, done);
//     VERBOSE("done: " << done);
//
//     // attempt to wait for client but timeout
//     // because nothing arrives
//     std::string latemsg = "- NO MESSAGE -";
//     std::getline(connection, latemsg);
//     VERBOSE("Fail: " << connection.fail());
//     VERBOSE("Bad:  " << connection.bad());
//     VERBOSE(latemsg);
//
//     // shouldn't throw except for a system io error
//     // or if close was called before open, nevertheless
//     // try-catch for demo purposes
//     try { connection.close(); } catch (...) {}
//     try { server.close(); } catch (...) {}
//
//     VERBOSE("- END OF LINE -");
//
//     return 0;
// }

/**
 * Created by TekuConcept on July 8, 2018
 */

#include <iostream>
#include <string>
#include <io_error>
#include <basic_socket>
#include <socketstream>

#define VERBOSE(x) std::cout << x << std::endl
using namespace Impact;

int main() {
  VERBOSE("- BEGIN TCP DEMO -");

  try {
    basic_socket server = make_tcp_socket();
    server.local_port(25565);
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

    try { client.close(); } catch (...) {VERBOSE("client close error");}
    try { server.close(); } catch (io_error e) {
      VERBOSE("server close error " << e.what());
    }
  } catch(std::exception e) {
    VERBOSE("+ Error: " << e.what());
  }

  VERBOSE("- END OF TCP DEMO -");
  return 0;
}
