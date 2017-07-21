#include "TcpClient.h"
#include <iostream>

int main() {
    Impact::TcpClient client(25565);
    
    std::cout << "- CLIENT CONNECTING -" << std::endl;
    
    client << "Hello From Client" << std::endl;
    
    std::string msg;
    std::getline(client, msg);
    std::cout << "msg: " << msg << std::endl;
    
    client << "I got your message!" << std::endl;
    client.disconnect();
    
    std::cout << "- END OF LINE -" << std::endl;
    
    return 0;
}