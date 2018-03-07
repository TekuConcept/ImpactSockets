#include <TcpClient.h>
#include <TcpServer.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "StaticCommand.h"

#define VERBOSE(x) std::cout << x << std::endl
using namespace Impact;

void callback(Object&,EventArgs) {
    std::cerr << "Read Timed Out [Intentional]" << std::endl;
}

int main() {
    VERBOSE("- BEGIN DEMO -");
    TcpServer server(25565);
    VERBOSE("> Server started");
    
    
    std::thread clientThread = std::thread([](){
        TcpClient client(25565);
        VERBOSE("> Client started");
        std::this_thread::sleep_for(std::chrono::seconds(3));
        VERBOSE("> Client disconnecting ungracefully");
    });


    auto connection = server.accept();
    KeepAliveOptions opts = {
        .idle = 5,
        .interval = 1,
        .count = 2,
    };
    Socket::keepalive(connection->getHandle(), opts);
    VERBOSE("> Found new Client");
    // wait for client tear-down
    std::this_thread::sleep_for(std::chrono::seconds(3));


    std::cout << "> Message: ";
    for(int i = 0; i < 10; i++)
        std::cout << connection->get() << " ";
    std::cout << std::endl;
    VERBOSE("> Was the message 0s as expected?");

    
    std::cout << "> Checking connection...";
    auto flag = connection->isConnected();
    VERBOSE((flag?"still connected":"disconnected"));
    
    
    if(flag) {
        VERBOSE("> Breaking the pipe by sending data!");
        *connection << "Ping" << std::flush;
        
        std::cout << "> Checking connection...";
        flag = connection->isConnected();
        VERBOSE((flag?"still connected":"disconnected"));
    }
    
    
    if(flag) {
        std::cout << "> Closing connection...";
        connection->disconnect();
        VERBOSE("Done!");
    }
    
    
    clientThread.join();
    VERBOSE("- END OF LINE -");
    return 0;
}
