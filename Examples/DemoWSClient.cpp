#include <WsClient.h>
#include <iostream>
#include <sstream>

int main() {
  Impact::WsClient client(25565);

  std::cout << "- CLIENT CONNECTING -" << std::endl;

  client.write("Hello From Client\n");

  std::string msg;
  std::stringstream ss;
  char c = ' ';
  bool skip = true;
  do {
	  if(!skip)
		ss << c;
	  else skip = false;
  } while ((c = client.read()) != '\n');
  msg = ss.str();
  std::cout << "msg: " << msg << std::endl;

  client.write("I got your message!\n");
  client.disconnect();

  std::cout << "- END OF LINE -" << std::endl;

  return 0;
}
