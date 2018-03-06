/**
 * Created by TekuConcept on January 1, 2018
 * Note: This client is designed to emulate the
 * browser Websocket client test platform ("/WebsocketTestPlatform/index.html").
 */

#include <TcpClient.h>
#include <RFC/6455>
#include <iostream>
#include <string>
#include <sstream>

using namespace Impact;
using namespace RFC6455;

#define VERBOSE(x) std::cout << x << std::endl

void messenger(Websocket& ws) {
    std::string line;
    std::ostringstream ss;
    do {
        // varify we're still connected
        if(ws.getState() == STATE::CLOSED) break;
        
        // send a message
        std::getline(std::cin, line);
        ss.clear();
        ss.str("{\"message\":\"");
        ss << line << "\"}";
        ws.sendText(ss.str());
        
        // read a message
        auto packet = ws.read();
        if(packet.opcode == WS_OP_TEXT)
             std::cout << packet.data;
        else std::cout << "Op Code: " << (int)packet.opcode;
        std::cout << std::endl;
    } while(!(line == "quit" || line == "exit"));
}

int main() {
    VERBOSE("- CLIENT STARTED -");
    
    std::cout << "> Connecting...";
    URI uri("ws://localhost:8082");
    TcpClient client(uri.port(), uri.host());
    if(!client.isConnected()) {
        VERBOSE("Could not connect.");
        VERBOSE("- END OF LINE -");
        return 1;
    }
    VERBOSE("Done!");
    
    std::cout << "> Shaking hands...";
    WebsocketClientNode ws(client, uri);
    if(!ws.initiateHandshake() && !ws.acceptHandshake()) {
        client.disconnect();
        VERBOSE("Handshake failed.");
        VERBOSE("- END OF LINE -");
        return 1;
    }
    VERBOSE("Done!");
    
    messenger(ws);
    client.disconnect();
    
    VERBOSE("- END OF LINE -");
    return 0;
}