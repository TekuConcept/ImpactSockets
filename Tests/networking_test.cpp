#include <iostream>
#include <iomanip>
#include <stdexcept>

#include <networking>

#define VERBOSE(x) std::cout << x << std::endl

using namespace impact;
using namespace networking;

void print_interfaces(const std::vector<netinterface>& list) {
	for (const auto& iface : list) {
		VERBOSE("Name:      " << iface.name);
		VERBOSE("Address:   " << iface.address);
		VERBOSE("Netmask:   " << iface.netmask);
		VERBOSE("Broadcast: " << iface.broadcast);
		VERBOSE("Flags:     " << iface.flags);

    std::cout << "MAC:       ";
		std::cout << std::hex;
		for (size_t i = 0; i < iface.mac.size() - 1; i++) {
			std::cout << std::setw(2) << std::setfill('0');
			std::cout << (unsigned int)iface.mac[i] << ':';
		}
		std::cout << std::setw(2) << std::setfill('0');
		std::cout << (unsigned int)iface.mac[iface.mac.size() - 1];
		VERBOSE(std::dec);

		VERBOSE("IPv4:      " << (iface.ipv4?"true":"false"));

		switch (iface.type) {
		case interface_type::OTHER:
			VERBOSE("Type:      Other");                      break;
		case interface_type::ETHERNET:
			VERBOSE("Type:      Ethernet");                   break;
		case interface_type::WIFI:
			VERBOSE("Type:      WiFi");                       break;
		case interface_type::FIREWIRE:
			VERBOSE("Type:      Firewire");                   break;
		case interface_type::PPP:
			VERBOSE("Type:      Point-to-Point Protocol");    break;
		case interface_type::ATM:
			VERBOSE("Type:      Asynchronous Transfer Mode"); break;
		default:
			VERBOSE("Type:      [unknown]");                  break;
		}

		std::cout << std::endl;
	}
}

int main() {
    VERBOSE("- BEGIN NETWORKING TEST -");

	try {
		std::vector<netinterface> list =
			networking::find_network_interfaces();
		print_interfaces(list);
	}
	catch (std::runtime_error e) {
		VERBOSE(e.what());
	}
	catch (...) {
		VERBOSE("Unexpected error");
	}

    VERBOSE("- END OF LINE -");
    return 0;
}
