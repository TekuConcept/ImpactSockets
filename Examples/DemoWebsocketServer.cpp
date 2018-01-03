/**
 * Created by TekuConcept on January 1, 2018
 * Note: This server is designed to emulate the
 * Node Websocket Server test platform ("/WebsocketTestPlatform/index.js").
 */

#include <TcpServer.h>
#include <RFC/6455>
#include <iostream>
#include <string>
#include "StaticCommand.h"

using namespace Impact;
using namespace RFC6455;

int main() {
    TcpServer server(8082);

    std::cout << "- SERVER STARTED -" << std::endl;

    auto connection = server.accept();
    
    // pass the new socket connection to the websocket server protocol wrapper
    WebsocketServerNode ws((std::iostream&)(*connection));
    
    if(ws.initiateHandshake()) {
        std::cout << "Websocket Message: new connection" << std::endl;
        ws.sendText("{\"message\":\"Hello Client\"}");
        // get a packet of information
        auto packet = ws.read();
        if(packet.opcode == WS_OP_TEXT) {
            std::cout << packet.data;
        }
        else {
            std::cout << "Op Code: " << (int)packet.opcode;
        }
        std::cout << std::endl;
        ws.close();
        connection->disconnect();
    }
    else {
        std::cout << "Websocket Message: handshake failed" << std::endl;
    }
    
    return 0;
}