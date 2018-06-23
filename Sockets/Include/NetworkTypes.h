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

		// !- PLATFORM-SPECIFIC TYPES -!
	// #if defined(_MSC_VER)
	// 	// #include <iphlpaoi.h>
	// 	// #pragma comment(lib, "IPHLPAPI.lib")
	// 	ETHERNET  = IF_TYPE_ETHERNET_CSMACD,
	// 	PPP       = IF_TYPE_PPP, /* Point-to-Point Protocol */
	// 	LOOPBACK  = IF_TYPE_SOFTWARE_LOOPBACK,
	// 	WIFI      = IF_TYPE_IEEE80211,
	// 	FIREWIRE  = IF_TYPE_IEEE1394,
	// 	ATM       = IF_TYPE_ATM,
	// 	TUNNEL    = IF_TYPE_TUNNEL,
	// 	TOKENRING = IF_TYPE_ISO88025_TOKENRING,
	// 	OTHER     = IF_TYPE_OTHER,
	// #elif defined(__APPLE__)
	// 	// #include <SystemConfiguration/SystemConfiguration>
	// 	// '-framework SystemConfiguration'
	// 	SIX2FOUR  = kSCNetworkInterfaceType6to4,
	// 	BLUETOOTH = kSCNetworkInterfaceTypeBluetooth,
	// 	BOND      = kSCNetworkInterfaceTypeBond,
	// 	ETHERNET  = kSCNetworkInterfaceTypeEthernet,
	// 	FIREWIRE  = kSCNetworkInterfaceTypeFireWire,
	// 	WIFI      = kSCNetworkInterfaceTypeIEEE80211,
	// 	IPSEC     = kSCNetworkInterfaceTypeIPSec,
	// 	IrDA      = kSCNetworkInterfaceTypeIrDA,
	// 	L2TP      = kSCNetworkInterfaceTypeL2TP,
	// 	MODEM     = kSCNetworkInterfaceTypeModem,
	// 	PPP       = kSCNetworkInterfaceTypePPP,
	// 	PPTP      = kSCNetworkInterfaceTypePPTP,
	// 	SERIAL    = kSCNetworkInterfaceTypeSerial,
	// 	VLAN      = kSCNetworkInterfaceTypeVLAN,
	// 	WWAN      = kSCNetworkInterfaceTypeWWAN,
	// 	IPV4      = kSCNetworkInterfaceTypeIPv4,
	// 	// LOOPBACK  = kSCNetworkInterfaceTypeLoopback,
	// 	// BRIDGE    = kSCNetworkInterfaceTypeBridge,
	// 	// VPN       = kSCNetworkInterfaceTypeVPN,
	// #else /* linux */
	// 	// #include <net/if_arp.h>
	// 	NET_ROM   = ARPHRD_NETROM,     // 0
	// 	ETHERNET  = ARPHRD_ETHER,      // 1
	// 	EETHERNET = ARPHRD_EETHER,     // 2 experimental
	// 	AX25      = ARPHRD_AX25,       // 3
	// 	PRONET    = ARPHRD_PRONET,     // 4
	// 	CHAOS     = ARPHRD_CHAOS,      // 5
	// 	WIFI      = ARPHRD_IEEE802,    // 6
	// 	ARCNET    = ARPHRD_ARCNET,     // 7
	// 	APPLE_TLK = ARPHRD_APPLETLK,   // 8
	// 	DLCI      = ARPHRD_DLCI,       // 15
	// 	ATM       = ARPHRD_ATM,        // 19
	// 	METRICOM  = ARPHRD_METRICOM,   // 23
	// 	FIREWIRE  = ARPHRD_IEEE1394,   // 24
	// 	EUI64     = ARPHRD_EUI64,      // 27
	// 	INFINIBAND= ARPHRD_INFINIBAND, // 32

	// 	SLIP      = ARPHRD_SLIP,       // 256
	// 	CSLIP     = ARPHRD_CSLIP,      // 257
	// 	SLIP6     = ARPHRD_SLIP6,      // 258
	// 	CSLIP6    = ARPHRD_CSLIP6,     // 259
	// 	RESERVED  = ARPHRD_RSRVD,      // 260
	// 	ADAPT     = ARPHRD_ADAPT,      // 264
	// 	ROSE      = ARPHRD_ROSE,       // 270
	// 	X25       = ARPHRD_X25,        // 271
	// 	HWX25     = ARPHRD_HWX25,      // 272
	// 	PPP       = ARPHRD_PPP,        // 512 /* Point-to-Point Protocol */
	// 	CISCO     = ARPHRD_CISCO,      // 513
	// 	HDLC      = ARPHRD_CISCO,      // 513
	// 	LAPB      = ARPHRD_LAPB,       // 516
	// 	DDCMP     = ARPHRD_DDCMP,      // 517
	// 	RAW_HDLC  = ARPHRD_RAWHDLC,    // 518
	// 	TUNNEL    = ARPHRD_TUNNEL,     // 768
	// 	TUNNEL6   = ARPHRD_TUNNEL6,    // 769
	// 	FRAME_RELAY_ACCESS_DEVICE = ARPHRD_FRAD, // 770
	// 	SKIP      = ARPHRD_SKIP,       // 771
	// 	LOOPBACK  = ARPHRD_LOOPBACK,   // 772
	// 	LOCAL_TALK = ARPHRD_LOCALTLK,  // 773
	// 	FIBER_DIST_DATA_IFACE = ARPHRD_FDDI, // 774
	// 	BIF       = ARPHRD_BIF,        // 775
	// 	SIT       = ARPHRD_SIT,        // 776
	// 	IP_DDP    = ARPHRD_IPDDP,      // 777
	// 	IP_GRE    = ARPHRD_IPGRE,      // 778
	// 	PIMSM_REG_IFACE = ARPHRD_PIMREG, // 779
	// 	HIGH_PERFORMANCE_PARALLEL_IFACE = ARPHRD_HIPPI, // 780
	// 	ASH       = ARPHRD_ASH,        // 781
	// 	ECONET    = ARPHRD_ECONET,     // 782
	// 	IRDA      = ARPHRD_IRDA,       // 783
	// 	PP_FIBER  = ARPHRD_FCPP,       // 784
	// 	AL_FIBER  = ARPHRD_FCAL,       // 785
	// 	PL_FIBER  = ARPHRD_FCPL,       // 786
	// 	FABRIC_FIBER = ARPHRD_FCFABRIC, // 787
	// 	WIFI_MAGIC = ARPHRD_IEEE802_TR, // 800
	// 	WIFI2     = ARPHRD_IEEE80211,  // 801
	// 	WIFI_PRISM = ARPHRD_IEEE80211_PRISM, // 802
	// 	WIFI_RADIO = ARPHRD_IEEE80211_RADIOTAP, // 803
	// 	LR_WPAN   = ARPHRD_IEEE802154, // 804
	// 	LR_WPAN_PHY = ARPHRD_IEEE802154_PHY, // 805
	// 	VOID      = ARPHRD_VOID, // 0xFFFF - unknown
	// 	NONE      = ARPHRD_NONE, // 0xFFFE - zero-length header
	// #endif
	} InterfaceType;
}

#endif