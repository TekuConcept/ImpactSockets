#include <iostream>
#include <iomanip>
#include <stdexcept>

#include <networking>

#define VERBOSE(x) std::cout << x << std::endl

using namespace impact;
using namespace networking;

void
print_interfaces(const std::vector<netinterface>& list)
{
    for (const auto& iface : list) {
        VERBOSE("Name:      " << iface.name);
		VERBOSE("Friendly:  " << iface.friendly_name);
		VERBOSE("Index:     " << iface.index);
        VERBOSE("Address:   " << networking::sockaddr_to_string(iface.address.get()));
        VERBOSE("Netmask:   " << networking::sockaddr_to_string(iface.netmask.get()));
        VERBOSE("Broadcast: " << networking::sockaddr_to_string(iface.broadcast.get()));
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

        VERBOSE("IPv4:      " << (iface.ipv4 ? "true" : "false"));
		VERBOSE("IPv6:      " << (iface.ipv6 ? "true" : "false"));

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


void
test_find_network_interfaces()
{
    VERBOSE("[Testing] Find Network Interface");
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
}


void
test_find_default_route()
{
    VERBOSE("[Testing] Find Default Route");
    struct netroute route = networking::find_default_route();
    VERBOSE("Interface Name:  " << route.name);
	VERBOSE("Interface Index: " << route.iface_index);
    VERBOSE("Gateway:         " << networking::sockaddr_to_string(route.gateway.get()));
}


int main() {
    VERBOSE("- BEGIN NETWORKING TEST -");

    test_find_network_interfaces();
    test_find_default_route();

    VERBOSE("- END OF LINE -");
    return 0;
}
