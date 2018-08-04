#include <iostream>
#include <stdexcept>
#include <algorithm> /* For transform() */
#include <thread>
#include <chrono>
#include <future>    /* async() */
#include <atomic>

#include <basic_socket>
#include <impact_error>
#include <networking>

using namespace impact;

#define VERBOSE(x) std::cout << x << std::endl
#define CATCH(x) \
	try { x }\
	catch (impact_error e) { VERBOSE(e.what()); exit(1); }\
	catch (...) { VERBOSE("Unknown internal error"); exit(2); }

using Interface     = networking::netinterface;
using InterfaceType = networking::InterfaceType;


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
		if (!iface.ipv4)                            continue;
		if (!(iface.type == InterfaceType::WIFI ||
            iface.type == InterfaceType::ETHERNET)) continue;
		if (iface.broadcast.size() == 0)            continue;

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
				VERBOSE("Sending to " << iface.broadcast << ":25565");
				socket.sendto(&message[0], message.length(), 25565, iface.broadcast);
			}
		}
	}
	catch (impact_error e) {
		VERBOSE(e.what());
		throw;
	}
}


void receiveResponse(basic_socket& socket) {
	try {
		struct poll_handle handle;
		handle.socket = socket.get();
		handle.events = (int)PollFlags::IN;
		std::vector<poll_handle> handles{ handle };

		const int kTimeout = 1000;
		const int kLength  = 512;
		const int kRetries = 5;
		char buffer[kLength];

		for (auto retries = 0; retries < kRetries;) {
			std::string    address;
			unsigned short port;
			int            size;

			// timeout
			auto status = impact::poll(&handles, kTimeout);
			if (status == 0)
				size = 0;
			else {
				auto flags = handles[0].return_events;
				handles[0].return_events = 0;

				if ((int)(flags & (int)PollFlags::IN))
					size = socket.recvfrom(buffer, kLength, &port, &address);
				else size = 0;
			}

			if (size == kLength)
				VERBOSE("[ overflow ]");
			else if (size > 0) {
				retries      = 0;
				buffer[size] = 0;
				VERBOSE("Received from: " << address << ":" << port << "\t" << buffer);
			}
			else retries++;
		}
	}
	catch (...) { throw; }
}


void runClient() {
	try {
		std::vector<Interface> list = networking::find_network_interfaces();

		list = filter(list);
		printInterfaces(list);

		basic_socket socket = make_udp_socket();
		socket.broadcast(true);
		socket.reuse_address(true);
		socket.multicast_ttl(2);
		VERBOSE("> Bind");
		socket.bind("0.0.0.0", 25565);

		VERBOSE("> Sending");
		sendMessage(socket, list);

		VERBOSE("> Receiving");
		receiveResponse(socket);

		VERBOSE("> Done!");
		socket.close();
	}
	catch (impact_error e) { VERBOSE("CLIENT: " << e.what()); }
	catch (...) { VERBOSE("CLIENT: Unknown error"); }
}

int main() {
	VERBOSE("- BEGINING NETWORK DISCOVERY -");

	runClient();

	VERBOSE("- END OF LINE -");
	return 0;
}
