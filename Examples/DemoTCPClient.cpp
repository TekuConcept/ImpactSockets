#include <TcpSocket.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <stdexcept>

#define VERBOSE(x) std::cout << x << std::endl

int main() {
    Impact::TcpSocket client;

    client.open(25565);
    if(client.fail()) {
        VERBOSE("Could not open socket connection.");
        return 1;
    }
    
    VERBOSE("- CLIENT CONNECTING -");
    
    client << "Hello From Client" << std::endl;
    
    std::string msg;
    if (!std::getline(client, msg)) {
        VERBOSE("Could not get server message.");
        VERBOSE("- END OF LINE -");
        return 1;
    }
    VERBOSE("msg: " << msg);
    
    client << "I got your message!" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    client << "...sorry I was late" << std::endl;
    
    // shouldn't throw except for a system io error
    // or if close was called before open, nevertheless
    // try-catch for demo purposes
    try { client.close(); } catch (...) {}
    
    VERBOSE("- END OF LINE -");
    
    return 0;
}
