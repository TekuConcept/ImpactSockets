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

using namespace Impact;
using namespace RFC6455;

#define VERBOSE(x) std::cout << x << std::endl

std::atomic<bool> shutdown;

void close() {
    while(!shutdown) {
        // check console override once every second
        if(std::cin) {
            std::string line;
            // This will block until \n is received;
            // from the terminal, user input almost always ends
            // with \n but program/console input might not.
            std::getline(std::cin, line);
            if(line == "quit" || line == "exit") {
                shutdown = true;
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    TcpServer server(8082);
    std::thread service;
    shutdown = false;

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
            while(!shutdown) {
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
                    shutdown = true;
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
    
    close();
    service.join();
    VERBOSE("- END OF LINE -");
    return 0;
}