/**
 * Created by TekuConcept on January 1, 2018
 * Note: This server is designed to emulate the
 * Node Websocket Server test platform ("/WebsocketTestPlatform/index.js").
 */

#include <TcpServer.h>
#include <RFC/6455>
#include <iostream>
#include <string>
#include <sstream>

#include <atomic>
#include <thread>
#include <chrono>
#include <csignal>

using namespace Impact;
using namespace RFC6455;

#define VERBOSE(x) std::cout << x << std::endl

std::atomic<bool> shutingdown;

void signalHandler( int signum ) {
    VERBOSE("Interrupt signal (" << signum << ") received.");

    // immediate shutdown and cleanup

    VERBOSE("- END OF LINE -");
    exit(signum);  
}

int main() {
    signal(SIGINT, signalHandler);
    
    TcpServer server(8082);
    std::thread service;
    shutingdown = false;

    VERBOSE("- SERVER STARTED -");

    auto connection = server.accept();
    // pass the new socket connection to the websocket server protocol wrapper
    WebsocketServerNode ws((std::iostream&)(*connection));
    VERBOSE("> New connection accepted");
    
    if(ws.initiateHandshake()) {
        VERBOSE("> Websocket handshake accepted");
        service = std::thread([&](){
            std::ostringstream msg;
            unsigned int count = 0;
            while(!shutingdown) {
                // send a packet of information
                msg.clear();
                msg.str("{\"message\":\"Hello Client ");
                msg << count << "\"}";
                ws.sendText(msg.str());
                
                // get a packet of information
                auto packet = ws.read();
                if(packet.opcode == WS_OP_TEXT)
                     std::cout << packet.data;
                else std::cout << "Op Code: " << (int)packet.opcode;
                std::cout << std::endl;
                
                if(packet.opcode == WS_OP_CLOSE)
                    shutingdown = true;
            }
            
            std::cout << "> Websocket disconnecting...";
            ws.close();
            connection->disconnect();
            VERBOSE("Done!");
        });
    }
    else {
        VERBOSE("> Websocket handshake failed");
        return 0;
    }
    
    service.join();
    VERBOSE("- END OF LINE -");
    return 0;
}