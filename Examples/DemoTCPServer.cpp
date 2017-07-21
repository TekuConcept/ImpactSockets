#include "TcpServer.h"
#include <iostream>
#include <string>

int main() {
    Impact::TcpServer server(25565);
    
    std::cout << "- SERVER STARTED -" << std::endl;
    
    auto connection = server.accept();
    std::cout << "Found new Client" << std::endl;
    
    std::string msg;
    std::getline(*connection, msg);
    std::cout << "msg: " << msg << std::endl;

    *connection << "Hello From Server" << std::endl;
    
    std::string done;
    std::getline(*connection, done);
    std::cout << "done: " << done << std::endl;
    
    std::cout << "- END OF LINE -" << std::endl;
    
    return 0;
}