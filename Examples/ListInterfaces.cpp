#include <iostream>
#include <iomanip>
#include <string>

#include <impact_error>
#include <networking>

#define VERBOSE(x) std::cout << x << std::endl

using namespace impact;
using iface      = networking::netinterface;
using iface_type = networking::interface_type;


std::string
interface_type_to_string(iface_type __type)
{
    switch (__type) {
        case iface_type::OTHER:    return "Other";
        case iface_type::ETHERNET: return "Ethernet";
        case iface_type::WIFI:     return "WiFi";
        case iface_type::FIREWIRE: return "Firewire";
        case iface_type::PPP:      return "Point-to-Point Protocol";
        case iface_type::ATM:      return "Asynchronous Transfer Mode";
        default:                   return "[unknown]";
    }
}


void
print_mac(const iface& __iface)
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
print_interfaces(const std::vector<iface>& __list)
{
    for (const auto& iface : __list) {
        VERBOSE("Name:      " << iface.name);
        VERBOSE("Address:   " << networking::sockaddr_to_string(iface.address.get()));
        VERBOSE("Netmask:   " << networking::sockaddr_to_string(iface.netmask.get()));
        VERBOSE("Broadcast: " << networking::sockaddr_to_string(iface.broadcast.get()));
        VERBOSE("Flags:     " << iface.flags);
        VERBOSE("IPv4:      " << (iface.ipv4 ? "true" : "false"));
        VERBOSE("IPv6:      " << (iface.ipv6 ? "true" : "false"));
        VERBOSE("Type:      " << interface_type_to_string(iface.type));
        print_mac(iface);
        std::cout << std::endl;
    }
}


int
main()
{
    VERBOSE("- BEGIN -");
    
    std::vector<iface> list = networking::find_network_interfaces();
    print_interfaces(list);

    VERBOSE("- END OF LINE -");
    return 0;
}