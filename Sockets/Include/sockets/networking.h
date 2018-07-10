/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef _NETWORKING_H_
#define _NETWORKING_H_

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
		unsigned int   flags;
		std::string    name;
		std::string    address;
		std::string    netmask;
		std::string    broadcast;
		interface_type type;
		bool           ipv4;

		netinterface();
	} NetworkInterface;

	
	std::vector<struct netinterface> find_network_interfaces()
		/* throw(std::runtime_error) */;
}}

#endif