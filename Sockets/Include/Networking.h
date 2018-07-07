/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef _NETWORKING_H_
#define _NETWORKING_H_

#include <string>
#include <vector>

namespace Impact {
namespace Networking {
    typedef enum class InterfaceType {
		// !- CROSS-PLATFORM TYPES    -!
		OTHER,
		ETHERNET,
		WIFI,
		FIREWIRE,
		PPP,
		ATM
	} InterfaceType;
	
	
	typedef struct NetInterface {
		unsigned int flags;
		std::string name;
		std::string address;
		std::string netmask;
		std::string broadcast;
		InterfaceType type;
		bool ipv4;
		NetInterface();
	} NetInterface;
	
	
	std::vector<NetInterface> findNetworkInterfaces()
			/* throw(std::runtime_error) */;
}}

#endif