/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef _IMPACT_NETWORKING_H_
#define _IMPACT_NETWORKING_H_

#include <string>
#include <vector>

namespace impact {
namespace networking {
	typedef enum class interface_type {
		// !- CROSS-PLATFORM TYPES    -!
		OTHER,
		ETHERNET,
		WIFI,
		FIREWIRE,
		PPP,
		ATM
	} InterfaceType;


	typedef struct netinterface {
		std::string                name;
		interface_type             type;
		unsigned int               flags;
		std::vector<unsigned char> address;
		std::vector<unsigned char> netmask;
		std::vector<unsigned char> broadcast;
		std::vector<unsigned char> mac;
		bool                       ipv4 : 1;
		bool                       ipv6 : 1;
		netinterface();
	} NetworkInterface;


	/**
	 * BUG: Only the second three bytes of the MAC address is saved
	 * as the first three bytes in the netinterface struct.
	 * PLATFORM: OSX
	 * REPRODUCE: Compare raw ARP ping to target MAC with the MAC
	 * returned by find_network_interfaces().
	 */
	std::vector<struct netinterface> find_network_interfaces()
		/* throw(std::runtime_error) */;
}}

#undef MAC_ADDRESS_LENGTH
#endif
