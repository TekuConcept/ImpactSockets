#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>

#include <networking>
#include <sockets/raw_socket.h>

#define VERBOSE(x) std::cout << x << std::endl

using namespace impact;

int main() {
    VERBOSE("- BEGIN ETHERNET DEMO -");

    std::vector<networking::netinterface> interfaces =
        networking::find_network_interfaces();


    /* select interface */
    networking::netinterface selected_iface;
    for (const auto& iface : interfaces) {
        if (!iface.ipv4) continue; /* select IPv4 only */
        if (iface.friendly_name == "WiFi"  || iface.friendly_name == "Wi-Fi" ||
            iface.friendly_name == "wlan0" || iface.friendly_name == "eth0" ||
            iface.friendly_name == "en1") {
            selected_iface = iface;
            break;
        }
    }
    if (selected_iface.name.size() == 0) {
        VERBOSE("Can't find interface for this demo...");
        VERBOSE("- END OF ETHERNET DEMO -");
        return 1;
    }
    VERBOSE("Selecting Interface: " << selected_iface.name <<
        " (" << selected_iface.friendly_name << ")");


    /* create and associate raw socket to interface */
    experimental::raw_socket raw;
    raw.attach(selected_iface.name);
    std::vector<unsigned char> buffer;
    buffer.resize(2048);
    VERBOSE("Buffer size: " << buffer.size());


    /* write packet */
    VERBOSE("Writing packet...");
    for (int i = 0; i < 6; i++)
        buffer[i] = 0x01; /* destination mac address */
    for (int i = 6; i < 12; i++)
        buffer[i] = 0x02; /* source mac address */
    buffer[12] = 0xC0; buffer[13] = 0xD0; /* packet type */ 
    /* 64 bytes is the minimum ether packet size */
    for (int i = 14; i < 64; i++)
        buffer[i] = 0x00;
    for (int i = 0; i < 5; i++)
        /* send enough packets to see the results on wireshark */
        raw.send(&buffer[0], 64);


    /* read packets */
    VERBOSE("Reading packets...");
    int result;
    std::cout << std::hex << std::setfill('0');
    while ((result = raw.recv(&buffer[0], buffer.size())) >= 0) {
        if (result == 0) continue;
        std::cout << "packet: ";
        for (int i = 0; i < result; i++)
            std::cout << std::setw(2) << (int)(buffer[i]) << " ";
        std::cout << std::endl;
    }

    VERBOSE("- END OF ETHERNET DEMO -");
    return 0;
}