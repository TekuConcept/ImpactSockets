/**
 * Created by TekuConcept on June 22, 2018
 */

#ifndef _NETWORK_TYPES_H_
#define _NETWORK_TYPES_H_

namespace Impact {
	typedef enum class InterfaceType {
		// !- CROSS-PLATFORM TYPES    -!
		OTHER,
		ETHERNET,
		WIFI,
		FIREWIRE,
		PPP,
		ATM
	} InterfaceType;
}

#endif
