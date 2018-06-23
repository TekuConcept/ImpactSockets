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
		std::cout << std::endl;
	}

	VERBOSE("- END OF LINE -");
	return 0;
}