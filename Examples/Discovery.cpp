#include <iostream>
#include <stdexcept>
#include <algorithm> /* For transform() */
#include <SocketInterface.h>

#define VERBOSE(x) std::cout << x << std::endl

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
	for (const auto& interface : list) {
		if(!interface.ipv4) continue;
		if(!(interface.type == InterfaceType::WIFI ||
			interface.type == InterfaceType::ETHERNET)) continue;
		if(interface.address.size() == 0) continue;
		std::string name = interface.name;
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);
		if(name.compare(0, 2, "en") == 0 ||
			name.compare(0, 3, "eth") == 0 ||
			name.compare(0, 4, "wlan") == 0 ||
			name.compare(0, 5, "wi-fi") == 0)
			table.push_back(interface);
	}

	return table;
}

int main() {
	VERBOSE("- BEGINING NETWORK DISCOVERY -");

	std::vector<NetInterface> list;
	try { list = Impact::SocketInterface::getNetworkInterfaces(); }
	catch (std::runtime_error e) {
		VERBOSE(e.what());
		return 1;
	}

	printInterfaces(filter(list));

	VERBOSE("- END OF LINE -");
	return 0;
}
