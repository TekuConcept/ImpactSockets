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
    
    // attempt to wait for client but timeout because nothing arrives
    std::string latemsg = "- NO MESSAGE -";
    std::getline(*connection, latemsg);
    std::cout << latemsg << std::endl;

    std::cout << "- END OF LINE -" << std::endl;
    
    return 0;
}