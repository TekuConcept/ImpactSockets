#include <iostream>
#include <stdexcept>
#include <algorithm> /* For transform() */
#include <thread>
#include <chrono>
#include <future>    /* async() */
#include <atomic>

#include <basic_socket>
#include <networking>

using namespace impact;

#define VERBOSE(x) std::cout << x << std::endl
#define CATCH(x) \
	try { x } catch (std::runtime_error e)\
	{ VERBOSE(e.what()); exit(1); }

using Interface     = networking::netinterface;
using InterfaceType = networking::InterfaceType;


std::thread service;
std::atomic<bool> shutdownServer(false);
poll_vector pollHandle;


void printInterfaces(std::vector<Interface> list) {
	for (const auto& iface : list) {
		VERBOSE("Name:      " << iface.name);
		VERBOSE("Address:   " << iface.address);
		VERBOSE("Netmask:   " << iface.netmask);
		VERBOSE("Broadcast: " << iface.broadcast);
		VERBOSE("Flags:     " << iface.flags);
		VERBOSE("IPv4:      " << (iface.ipv4?"true":"false"));

		switch (iface.type) {
		case InterfaceType::OTHER:
			VERBOSE("Type:      Other");                      break;
		case InterfaceType::ETHERNET:
			VERBOSE("Type:      Ethernet");                   break;
		case InterfaceType::WIFI:
			VERBOSE("Type:      WiFi");                       break;
		case InterfaceType::FIREWIRE:
			VERBOSE("Type:      Firewire");                   break;
		case InterfaceType::PPP:
			VERBOSE("Type:      Point-to-Point Protocol");    break;
		case InterfaceType::ATM:
			VERBOSE("Type:      Asynchronous Transfer Mode"); break;
		default:
			VERBOSE("Type:      [unknown]");                  break;
		}

		std::cout << std::endl;
	}
}


std::vector<Interface> filter(std::vector<Interface> list) {
	std::vector<Interface> table;

	// collect interfaces with the following characteristics
	for (const auto& iface : list) {
		if (!iface.ipv4)                              continue;
		if (!(iface.type == InterfaceType::WIFI ||
			    iface.type == InterfaceType::ETHERNET)) continue;
		if (iface.broadcast.size() == 0)              continue;

		std::string name = iface.name;
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);
		if (name.compare(0, 2, "en")    == 0 ||
			  name.compare(0, 3, "eth")   == 0 ||
			  name.compare(0, 4, "wlan")  == 0 ||
			  name.compare(0, 5, "wi-fi") == 0 ||
			  name.compare(0, 3, "wlp")   == 0)
			table.push_back(iface);
	}

	return table;
}


void sendMessage(basic_socket& socket, std::vector<Interface> list) {
	try {
		std::string message("discovery-request");
		for (const auto& iface : list) {
			for (auto i = 0; i < 5; i++) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				VERBOSE("Sending to " << iface.broadcast << ":5001");
				socket.sendto(&message[0], message.length(), 5001, iface.broadcast);
			}
		}
	}
	catch (...) { throw; }
}


void receiveResponse(basic_socket& socket) {
	try {
		const int kTimeout = 1000;
		const int kLength  = 512;
		const int kRetries = 5;
		char buffer[kLength];

		for (auto retries = 0; retries < kRetries;) {
			std::string    address;
			unsigned short port;
			int            size;

			// timeout
			auto status = probe::poll(pollHandle, kTimeout);
			if (status == 0)
				size = 0;
			else {
				auto flags = pollHandle[0];
				pollHandle.reset_events();

				if ((int)(flags & PollFlags::IN))
					size = socket.recvfrom(buffer, kLength, port, address);
				else size = 0;
			}

			if (size == kLength)
				VERBOSE("[ overflow ]");
			else if (size > 0) {
				retries      = 0;
				buffer[size] = 0;
				VERBOSE(address << ":" << port << "\t" << buffer);
			}
			else retries++;
		}
	}
	catch (...) { throw; }
}


// void runServer() {
// 	service = std::thread([](){
// 		try {
// 			basic_socket socket = make_udp_socket();
// 			socket.local_address_port(5001, "0.0.0.0");
// 			socket.broadcast(true);
//
// 			VERBOSE("UDP Listening On Port 5001\n");
//
// 			const int kTimeout = 1000;
// 			const int kLength  = 512;
//
// 			char           buffer[kLength];
// 			std::string    address;
// 			unsigned short port;
// 			int            size;
//
// 			while (!shutdownServer) {
// 				size = socket.recvFrom(buffer, kLength, port, address, kTimeout);
// 				if (size == kLength)
// 					VERBOSE("[ overflow ]");
// 				else if (size > 0)
// 					VERBOSE("Received: " << buffer);
// 				else continue;
// 			}
//
// 			socket.close();
// 		}
// 		catch(std::runtime_error e) { VERBOSE(e.what()); }
// 	});
// }


void runClient() {
	try {
		std::vector<Interface> list = networking::find_network_interfaces();

		list = filter(list);
		printInterfaces(list);

		basic_socket socket = make_udp_socket();
		socket.broadcast(true);
		socket.multicast_ttl(2);
	#if !defined(__WINDOWS__)
		// Windows automatically listens on the given port
		// No need to manuall bind to inaddrany:port
		socket.local_address_port("0.0.0.0", 5001);
	#endif
		pollHandle.push_back({ socket, PollFlags::IN });

		sendMessage(socket, list);
		receiveResponse(socket);

		pollHandle.pop_back();
		socket.close();
	} catch (std::runtime_error e) { VERBOSE(e.what()); }
}

int main() {
	VERBOSE("- BEGINING NETWORK DISCOVERY -");

	// runServer();
	// give time for server to start
	// std::this_thread::sleep_for(std::chrono::seconds(1));
	runClient();

	// shutdownServer = true;
	// if(service.joinable()) service.join();

	VERBOSE("- END OF LINE -");
	return 0;
}
