#include <iostream>
#include <stdexcept>
#include <algorithm> /* For transform() */
#include <thread>
#include <chrono>
#include <future> /* async() */
#include <atomic>
#include <SocketInterface.h>
#include <UdpSocket.h>

#define VERBOSE(x) std::cout << x << std::endl
#define CATCH(x) \
	try { x } catch (std::runtime_error e)\
	{ VERBOSE(e.what()); exit(1); }

using NetInterface  = Impact::NetInterface;
using InterfaceType = Impact::InterfaceType;

void printInterfaces(std::vector<NetInterface> list) {
	for(const auto& iface : list) {
		VERBOSE("Name:      " << iface.name);
		VERBOSE("Address:   " << iface.address);
		VERBOSE("Netmask:   " << iface.netmask);
		VERBOSE("Broadcast: " << iface.broadcast);
		VERBOSE("Flags:     " << iface.flags);
		VERBOSE("IPv4:      " << (iface.ipv4?"true":"false"));

		switch (iface.type) {
		case Impact::InterfaceType::OTHER:
			VERBOSE("Type:      Other"); break;
		case Impact::InterfaceType::ETHERNET:
			VERBOSE("Type:      Ethernet"); break;
		case Impact::InterfaceType::WIFI:
			VERBOSE("Type:      WiFi"); break;
		case Impact::InterfaceType::FIREWIRE:
			VERBOSE("Type:      Firewire");  break;
		case Impact::InterfaceType::PPP:
			VERBOSE("Type:      Point-to-Point Protocol");  break;
		case Impact::InterfaceType::ATM:
			VERBOSE("Type:      Asynchronous Transfer Mode"); break;
		default:
			VERBOSE("Type:      [unknown]"); break;
		}

		std::cout << std::endl;
	}
}

std::vector<NetInterface> filter(std::vector<NetInterface> list) {
	std::vector<NetInterface> table;

	// collect interfaces with the following characteristics
	for (const auto& iface : list) {
		if(!iface.ipv4) continue;
		if(!(iface.type == InterfaceType::WIFI ||
			iface.type == InterfaceType::ETHERNET)) continue;
		if(iface.broadcast.size() == 0) continue;
		std::string name = iface.name;
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);
		if(name.compare(0, 2, "en") == 0 ||
			name.compare(0, 3, "eth") == 0 ||
			name.compare(0, 4, "wlan") == 0 ||
			name.compare(0, 5, "wi-fi") == 0)
			table.push_back(iface);
	}

	return table;
}

void sendMessage(Impact::UdpSocket& socket, std::vector<NetInterface> list) {
	std::string message("msc-discovery-request");
	for(const auto& iface : list) {
		// (void)list;
		for(auto i = 0; i < 5; i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			try {
				VERBOSE("Sending to " << iface.broadcast << ":5001");
				socket.sendTo(&message[0], message.length(),
					5001, iface.broadcast);
			}
			catch (std::runtime_error e) {
				VERBOSE(e.what());
				CATCH(socket.close();)
				exit(1);
			}
		}
	}
}

void receiveResponse(Impact::UdpSocket& socket) {
	const int TIMEOUT = 1000;
	const int LENGTH = 512;
	char buffer[LENGTH];
	for(auto retries = 0; retries < 5;) {
		std::string address;
		unsigned short port;
		int size;
		try { size = socket.recvFrom(buffer, LENGTH, port, address, TIMEOUT); }
		catch (std::runtime_error e) {
			VERBOSE(e.what());
			CATCH(socket.close();)
			exit(1);
		}
		if (size == LENGTH) VERBOSE("[ overflow ]");
		else if(size > 0) {
			retries = 0;
			buffer[size] = 0;
			VERBOSE(address << ":" << port << "\t" << buffer);
		}
		else retries++;
	}
}

std::thread service;
std::atomic<bool> fshutdown(false);
void runServer() {
	service = std::thread([](){
		Impact::UdpSocket socket;
		CATCH(socket.open(5001, "0.0.0.0");)

		try { socket.setBroadcast(true); }
		catch (std::runtime_error e) {
			VERBOSE(e.what());
			CATCH(socket.close();)
			exit(1);
		}

		VERBOSE("UDP Listening On Port 5001\n");

		const int TIMEOUT = 1000;
		const int LENGTH = 512;
		char buffer[LENGTH];
		while(!fshutdown) {
			std::string address;
			unsigned short port;
			int size;
			try { size = socket.recvFrom(
				buffer, LENGTH, port, address, TIMEOUT); }
			catch (std::runtime_error e) {
				VERBOSE(e.what());
				CATCH(socket.close();)
				exit(1);
			}
			if (size == LENGTH) VERBOSE("[ overflow ]");
			else if(size > 0) {
				VERBOSE("Received: " << buffer);
			}
			else continue;
		}

		CATCH(socket.close();)
	});
}

void runClient() {
	std::vector<NetInterface> list;
	CATCH(list = Impact::SocketInterface::getNetworkInterfaces();)

	list = filter(list);
	printInterfaces(list);

	Impact::UdpSocket socket;
	CATCH(socket.open(5001, "0.0.0.0");)

	try { socket.setBroadcast(true); }
	catch (std::runtime_error e) {
		VERBOSE(e.what());
		CATCH(socket.close();)
		exit(1);
	}

	try { socket.setMulticastTTL(2); }
	catch (std::runtime_error e) {
		VERBOSE(e.what());
		CATCH(socket.close();)
		exit(1);
	}

	sendMessage(socket, list);
	receiveResponse(socket);

	CATCH(socket.close();)
}

int main() {
	VERBOSE("- BEGINING NETWORK DISCOVERY -");

	//runServer(); // Fails to run on windows
	//std::this_thread::sleep_for(std::chrono::seconds(1));
	runClient();

	fshutdown = true;
	if(service.joinable()) service.join();
	
	VERBOSE("- END OF LINE -");
	return 0;
}
