#include "TcpServer.h"
#include <iostream>
#include <string>
#include "StaticCommand.h"

void callback(Object&,EventArgs) {
    std::cerr << "Read Timed Out [Intentional]" << std::endl;
}

int main() {
    Impact::TcpServer server(25565);
    
    std::cout << "- SERVER STARTED -" << std::endl;
    
    auto connection = server.accept();
    connection->setTimeout(2500); // 2.5 seconds
    connection->onTimeout += StaticCommandPtr(
        EventArgs,
        callback
    );
    
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
