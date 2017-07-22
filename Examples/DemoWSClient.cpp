#include <WsClient.h>
#include <iostream>
#include <sstream>

void print(Impact::WsClient& client) {
  std::string msg;
  std::stringstream ss;
  char c = ' ';
  bool skip = true;
  do {
	  if(!skip)
		  std::cerr << c;
	  else skip = false;
  } while ((c = client.read()) != '\n');
  std::cerr << "LF" << std::endl;
}

int main() {
  Impact::WsClient client(38107);
  print(client);

/*
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
*/
  client.disconnect();

  std::cout << "- END OF LINE -" << std::endl;

  return 0;
}
