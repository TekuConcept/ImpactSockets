/**
 * Created by TekuConcept on June 18, 2018
 */

#ifndef _SOCKET_TYPES_H_
#define _SOCKET_TYPES_H_

#if defined(_MSC_VER)
 	#include <ws2def.h>
#else
	// #include <sys/socket.h>
 	#include <arpa/inet.h>
#endif

namespace Impact {
	typedef enum class SocketDomain {
		// !- CROSS-PLATFORM DOMAINS    -!
		UNSPECIFIED = AF_UNSPEC,   // Address is unspecified
		INET        = AF_INET,     // IPv4 Internet protocols
		INET6       = AF_INET6,    // IPv6 Internet protocols
		
		// !- PLATFORM-SPECIFIC DOMAINS -!
	#if defined(_MSC_VER)
		IRDA        = AF_IRDA,     // Infrared Data Association (IrDA)
	#elif defined(__APPLE__)
		// see <sys/socket.h> for more address families
		UNIX        = AF_UNIX,     // local communication (pipes) (AF_LOCAL)
		ISO         = AF_ISO,      // ISO protocols
		LINK        = AF_LINK,     // link layer interface
		SYSTEM      = AF_SYSTEM,   // kernel event messages
	#else /* linux */
		// see <bits/socket.h> for more address families
		UNIX        = AF_UNIX,     // local communication (AF_LOCAL)
		NETLINK     = AF_NETLINK,  // kernel user interface device
		CRYPTO      = AF_ALG,      // interface to kernel crypto API
	#endif
	} SocketDomain;


	/*
		[-- LINUX --]
		SOCK_NONBLOCK		- Set the O_NONBLOCK file status flag on the new open
							  file description. Using this flag saves extra calls
							  to fcntl(2) to achieve the same result.

		[-- WINDOWS --]
		FIONBIO             - ioctlsocket(descriptor, FIONBIO, enabled)
							  set non-blocking flag for given socket descriptor

		[-- APPLE --]
		F_SETFL				- fcntl(descriptor, F_SETFL, o_flags)
							  `#include <fcntl.h>`
							  o_flags &= ~O_NONBLOCK
	*/


	typedef enum class SocketType {
		STREAM            = SOCK_STREAM,   /* Provides sequenced, reliable, two-way,
							communication-based byte streams. An out-of-band data
							transmission mechanism may be supported.*/
		DATAGRAM          = SOCK_DGRAM,    /* Supports datagrams (connectionless,
							unreliable messages of a fixed maximum length).*/
		RAW               = SOCK_RAW,      /* Provides raw network protocol access. */
		RELIABLE_DATAGRAM = SOCK_RDM,      /* Provides a reliable datagram layer
							that does not guarantee ordering.*/
		SEQUENCED_PACKET  = SOCK_SEQPACKET /* Provides a sequenced, reliable,
							two-way connection-based data transmission path for
							datagrams of fixed maximum length; a consumer is
							required to read an entire packet with each input
							system call.*/
	} SocketType;


	typedef enum class SocketProtocol {
		// !-- CROSS-PLATFORM PROTOCOL SUPPORT --!
		DEFAULT = 0,

		IP            = IPPROTO_IP,       /* Dummy Protocol For TCP */
		ICMP          = IPPROTO_ICMP,     /* Internet Control Message Protocol */
		IGMP          = IPPROTO_IGMP,     /* Internet Group Management Protocol */
		TCP           = IPPROTO_TCP,      /* Transmission Control Protocol */
		EGP           = IPPROTO_EGP,      /* Exterior Gateway Protocol */
		PUP           = IPPROTO_PUP,      /* PARC Universal Packet Protocol */
		UDP           = IPPROTO_UDP,      /* User Datagram Protocol */
		IDP           = IPPROTO_IDP,      /* XNS IDP Protocol */
		IPV6          = IPPROTO_IPV6,     /* IPv6 Header */
		ESP           = IPPROTO_ESP,      /* Encapsulating Security Payload */
		AH            = IPPROTO_AH,       /* Authentication Header */
		PIM           = IPPROTO_PIM,      /* Protocol Independent Multicast */
		SCTP          = IPPROTO_SCTP,     /* Stream Control Transmission Protocol */
		RAW           = IPPROTO_RAW,      /* Raw IP Packets */
		// IPV6_HOPOPTS  = IPPROTO_HOPOPTS,  /* IPv6 Hop-by-Hop Options */
		// IPV6_ROUTING  = IPPROTO_ROUTING,  /* IPv6 Routing Header */
		// IPV6_FRAGMENT = IPPROTO_FRAGMENT, /* IPv6 Fragmentation Header */
		// IPV6_ICMPV6   = IPPROTO_ICMPV6,   /* Internet Control Message Protocol v6 */
		// IPV6_NONE     = IPPROTO_NONE,     /* IPv6 No Next Header */
		// IPV6_DSTOPTS  = IPPROTO_DSTOPTS,  /* IPv6 Destination Options */
		// MAX           = IPPROT_MAX,       /* maximum value of IPPROTO */

		// !-- PLATFORM-SPECIFIC PROTOCOL SUPPORT --!
	#if defined(_MSC_VER)
		// GGP           = IPPROTO_GGP,      /*m Gateway-to-Gateway Protocol */
		// IPV4          = IPPROTO_IPV4,     /*m IPv4 Encapsulation */
		// ST            = IPPROTO_ST,       /*m Stream Protocol */
		// CBT           = IPPROTO_CBT,      /* Core Based Trees Protocol */
		// IGP           = IPPROTO_IGP,      /* m Private Interior Gateway Protocol */
		// RDP           = IPPROTO_RDP,      /*m Reliable Data Protocol */
		// ND            = IPPROTO_ND,       /*m Net Disk Protocol */
		// ICLFXBM       = IPPROTO_ICLFXBM,  /* Wideband Monitoring */
		// PGM           = IPPROTO_PGM,      /*m Pragmatic General Multicast */
		// L2TP          = IPPROTO_L2TP,     /* Level 2 Tunneling Protocol */
	#elif defined(__APPLE__)
		// PIGP          = IPPROTO_PIGP,     /* Private Interior Gateway */
		// RCCMON        = IPPROTO_RCCMON,   /* BBN RCC Monitoring */
		// NVPII         = IPPROTO_NVPII,    /* Network Voice Protocol */
		// ARGUS         = IPPROTO_ARGUS,    /* IP Protocol 13 */
		// EMCON         = IPPROTO_EMCON,    /* Emission Control Protocol */
		// XNET          = IPPROTO_XNET,     /* Cross Net Debugger */
		// CHAOS         = IPPROTO_CHAOS,    /* Chaos */
		// MUX           = IPPROTO_MUX,      /* Multiplexing */
		// MEAS          = IPPROTO_MEAS,     /* DCN Measurement Subsystems */
		// HMP           = IPPROTO_HMP,      /* Host Monitoring */
		// PRM           = IPPROTO_PRM,      /* Packet Radio Measurement */
		// TRUNK1        = IPPROTO_TRUNK1,   /* Trunk-1 */
		// TRUNK2        = IPPROTO_TRUNK2,   /* Trunk-2 */
		// LEAF1         = IPPROTO_LEAF1,    /* Leaf-1 */
		// LEAF2         = IPPROTO_LEAF2,    /* Leaf-2 */
		// IRTP          = IPPROTO_IRTP,     /* Reliable Transaction */
		// BLT           = IPPROTO_BLT,      /* Bulk Data Transfer */
		// NSP           = IPPROTO_NSP,      /* Network Services */
		// INP           = IPPROTO_INP,      /* Merit Internodal */
		// SEP           = IPPROTO_SEP,      /* Sequential Exchange */
		// THIRD_PC      = IPPROTO_3PC,      /* Third Party Connect */
		// IDPR          = IPPROTO_IDPR,     /* InterDomain Policy Routing */
		// XTP           = IPPROTO_XTP,      /* Xpress Transport Protocol */
		// DDP           = IPPROTO_DDP,      /* Datagran Delivery */
		// CMTP          = IPPROTO_CMTP,     /* Control Message Transport */
		// TPXX          = IPPROTO_TPXX,     /* TP++ Transport */
		// IL            = IPPROTO_IL,       /* Internal Link Transport Protocol */
		// SDRP          = IPPROTO_SDRP,     /* Source Demand Routing */
		// IDRP          = IPPROTO_IDRP,     /* InterDomain Routing */
		// MHRP          = IPPROTO_MHRP,     /* Mobile Host Routing */
		// BHA           = IPPROTO_BHA,      /* Boston Housing Authority */
		// INLSP         = IPPROTO_INLSP,    /* Integrated Net Layer Security */
		// SWIPE         = IPPROTO_SWIPE,    /* IP With Encryption */
		// NHRP          = IPPROTO_NHRP,     /* Next Hop Resolution */
		// AHIP          = IPPROTO_AHIP,     /* Any Host Internal Protocol */
		// CFTP          = IPPROTO_CFTP,     /* Clients-Oriented File Transfer Protocol */
		// HELLO         = IPPROTO_HELLO,    /* "hello" Routing Protocol */
		// SATEXPAK      = IPPROTO_SATEXPAK, /* SATNET/BackRoom EXPAK */
		// KRYPTOLAN     = IPPROTO_KRYPTOLAN,/* Kryptolan */
		// RVD           = IPPROTO_RVD,      /* Remote Virtual Disk */
		// IPPC          = IPPROTO_IPPC,     /* Pluribus Packet Core */
		// ADFS          = IPPROTO_ADFS,     /* Any Distributed FS */
		// SATMON        = IPPROTO_SATMON,   /* Satnet Monitoring */
		// VISA          = IPPROTO_VISA,     /* VISA Protocol */
		// IPCV          = IPPROTO_IPCV,     /* Packet Core Utility */
		// CPNX          = IPPROTO_CPNX,     /* Computer Protocol Network Executive */
		// CPHP          = IPPROTO_CPHP,     /* Computer Protocol Heart Beat */
		// WSN           = IPPROTO_WSN,      /* Wang Span Network */
		// PVP           = IPPROTO_PVP,      /* Packet Video Protocol */
		// BRSATMON      = IPPROTO_BRSATMON, /* BackRoom SATNET Monitoring */
		// WBMON         = IPPROTO_WBMON,    /* Wideband Monitoring */
		// WBEXPAK       = IPPROTO_WBEXPAK,  /* Wideband EXPAK */
		// EON           = IPPROTO_EON,      /* ISO CNLP */
		// VMTP          = IPPROTO_VMTP,     /* Versatile Message Transaction Protocol */
		// SVMTP         = IPPROTO_SVMTP,    /* Secure Versatile Message Transaction Protocol */
		// VINES         = IPPROTO_VINES,    /* Banyon VINES */
		// TTP           = IPPROTO_TTP,      /* IP Protocol 84 */
		// DGP           = IPPROTO_DGP,      /* Dissimilar Gateway Protocol */
		// TCF           = IPPROTO_TCF,      /* Target Communicatio Framework */
		// IGRP          = IPPROTO_IGRP,     /* Cisco/GXS Interior Gateway Routing Protocol */
		// OSPFIGP       = IPPROTO_OSPFIGP,  /* Open Shortest Path First - Interior Gateway Protocol */
		// SRPC          = IPPROTO_SRPC,     /* Strite RPC Protocol */
		// LARP          = IPPROTO_LARP,     /* Locus Address Resolution */
		// AX25          = IPPROTO_AX25,     /* AX.25 Frames */
		// IPEIP         = IPPROTO_IPEIP,    /* IP Encapsulated In IP */
		// MICP          = IPPROTO_MICP,     /* Mobile Internetworking Control Protocol */
		// SCCSP         = IPPROTO_SCCSP,    /* Semaphore Communication Security Protocol */
		// ETHERIP       = IPPROTO_ETHERIP,  /* Ethernet IP Encapsulation */
		// APES          = IPPROTO_APES,     /* Any Private Encryption Scheme */
		// GMTP          = IPPROTO_GMTP,     /* Graphical Media Transfer Protocol */
		// IPCOMP        = IPPROTO_IPCOMP,   /* Payload Compression (IPComp) */
	#else /* linux */
		// IPIP          = IPPROT_IPIP,      /*m IPIP tunnels */
		// TP            = IPPROT_TP,        /*m SO Transport Protocol Class 4 */
		// DCCP          = IPPROT_DCCP,      /* Datagram Congestion Control Protocol */
		// RSVP          = IPPROT_RSVP,      /*m Reservation Protocol */
		// GRE           = IPPROT_GRE,       /*m General Routing Encapsulation */
		// MTP           = IPPROT_MTP,       /*m Multicast Transport Protocol */
		// BEETPH        = IPPROT_BEETPH,    /* IP Option Pseudo Header For BEET */
		// ENCAP         = IPPROT_ENCAP,     /*m Encapsulation Header */
		// COMP          = IPPROT_COMP,      /* Compression Header Protocol */
		// UDPLITE       = IPPROT_UDPLITE,   /* UDP-Lite Protocol */
		// MPLS          = IPPROT_MPLS,      /* Multi-Protocol Label Switching in IP */
		// IPV6_MH       = IPPROT_MH,        /* IPv6 Mobility Header */
	#endif
	} SocketProtocol;
}

#endif