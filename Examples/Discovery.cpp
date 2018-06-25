#include <iostream>
#include <SocketInterface.h>

#define VERBOSE(x) std::cout << x << std::endl

int main() {
	VERBOSE("- BEGINING NETWORK DISCOVERY -");

	auto list = Impact::SocketInterface::getNetworkInterfaces();
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

	VERBOSE("- END OF LINE -");
	return 0;
}
