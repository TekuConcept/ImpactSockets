#include "TcpClient.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    Impact::TcpClient client(25565);
    
    std::cout << "- CLIENT CONNECTING -" << std::endl;
    
    client << "Hello From Client" << std::endl;
    
    std::string msg;
	if (!std::getline(client, msg)) {
		std::cout << "Couldn't connect to local server..." << std::endl;
		std::cout << "- END OF LINE -" << std::endl;
		return 1;
	}
    std::cout << "msg: " << msg << std::endl;
    
    client << "I got your message!" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    client << "...sorry I was late" << std::endl;
    
    client.disconnect();
    
    std::cout << "- END OF LINE -" << std::endl;
    
    return 0;
}