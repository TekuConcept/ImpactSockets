#include <iostream>
#include <iomanip>
#include <string>

#include <impact_error>
#include <networking>

#define VERBOSE(x) std::cout << x << std::endl

using namespace impact;
using interface      = networking::netinterface;
using interface_type = networking::interface_type;


std::string
interface_type_to_string(interface_type __type)
{
    switch (__type) {
        case interface_type::OTHER:    return "Other";
        case interface_type::ETHERNET: return "Ethernet";
        case interface_type::WIFI:     return "WiFi";
        case interface_type::FIREWIRE: return "Firewire";
        case interface_type::PPP:      return "Point-to-Point Protocol";
        case interface_type::ATM:      return "Asynchronous Transfer Mode";
        default:                       return "[unknown]";
    }
}


void
print_mac(const interface& __iface)
{
    std::cout << std::setfill('0');
    std::cout << "MAC:       ";
    for (auto b : __iface.mac) {
        std::cout << std::setw(2) << std::hex;
        std::cout << (int)(0xFF & b) << " ";
    }
    std::cout << std::setfill(' ') << std::dec << std::endl;
}


void
print_interfaces(const std::vector<interface>& __list)
{
	for (const auto& iface : __list) {
		VERBOSE("Name:      " << iface.name);
		VERBOSE("Address:   " << iface.address);
		VERBOSE("Netmask:   " << iface.netmask);
		VERBOSE("Broadcast: " << iface.broadcast);
		VERBOSE("Flags:     " << iface.flags);
		VERBOSE("IPv4:      " << (iface.ipv4?"true":"false"));
		VERBOSE("Type:      " << interface_type_to_string(iface.type));
		print_mac(iface);
		std::cout << std::endl;
	}
}


int
main()
{
    VERBOSE("- BEGIN -");
    
    std::vector<interface> list = networking::find_network_interfaces();
    print_interfaces(list);
    
    VERBOSE("- END OF LINE -");
    return 0;
}