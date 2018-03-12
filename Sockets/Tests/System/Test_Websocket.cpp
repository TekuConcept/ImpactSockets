/**
 * Created by TekuConcept on March 9, 2018
 * 
 * System test against a node websocket server.
 * Tests:
 *   - Handshake
 *   - Data Frames (Text | Binary)
 *   - Fragmented Frames
 *   - Ping / Pong Frames
 *   - Close Frame
 */

#include <iostream>
#include <thread>
#include <chrono>

#include <RFC/2616> // URI
#include <RFC/6455>
#include <TcpClient.h>
#include <Websocket.h>
#include <wsmanip>

#define VERBOSE(x) std::cout << x << std::endl
#define DELAY_S(t) std::this_thread::sleep_for(std::chrono::seconds(t))

using namespace Impact;

int main() {
    VERBOSE("- BEGIN -");
    
    URI uri("ws://localhost:8080/");
    TcpClient socket(uri.port());
    socket.setTimeout(1000);
    Websocket web(socket,uri,WS_TYPE::WS_CLIENT);
    if(web.shakeHands()) {
        VERBOSE("> Handshake accepted");
        DELAY_S(1);
        
        // VERBOSE("> Sending non-fragmented data");
        // web << "Hello Connection" << ws::send;
        // DELAY_S(1);
        
        // VERBOSE("> Sending fragmented data");
        // web << "To be " << std::flush;
        // web << "continued." << ws::send;
        // DELAY_S(1);
        
        // VERBOSE("> Ping");
        // web << ws::ping; // no data
        // web << "Some " << std::flush;
        // web.ping("Are you alive?"); // with data
        // web << "data." << ws::send;
        // DELAY_S(1);
        
        VERBOSE("> Closing connection");
        web << ws::close;
        web.wait();
        DELAY_S(1);
    }
    else VERBOSE("> Handshake rejected");
    
    socket.disconnect();
    
    VERBOSE("- END OF LINE -");
    return 0;
}