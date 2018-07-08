/**
 * Created by TekuConcept on June 18, 2018
 */

#ifndef _SOCKET_TYPES_H_
#define _SOCKET_TYPES_H_

#include "Environment.h"

#if defined(__WINDOWS__)
 	#include <WinSock2.h>
 	#include <Ws2def.h>
	#include <Ws2ipdef.h>
	#include <Winerror.h>
#else
 	#include <sys/poll.h>
 	#include <arpa/inet.h>
 	#include <netdb.h>
 	#include <errno.h>
#endif

#if defined(__LINUX__)
 	#include <linux/version.h>
#endif

#if defined(__WINDOWS__)
	#pragma push_macro("IN")
	#undef IN
	#pragma push_macro("OUT")
	#undef OUT
	#pragma push_macro("ERROR")
	#undef ERROR
#elif defined(__clang__)
  #pragma push_macro("EOF");
  #undef EOF
#endif

#define ENUM_OPERATOR(T,U,O) \
inline T operator O (T lhs, T rhs) {\
	return static_cast<T>(static_cast<U>(lhs) O static_cast<U>(rhs));\
}

namespace Impact {
  typedef struct KeepAliveOptions {
		int enabled;  /* Enables KEEPALIVE on the target socket connection.  */
		int idleTime; /* Number of idle seconds before sending a KA probe.   */
		int interval; /* How often in seconds to resend an unacked KA probe. */
		int retries;  /* How many times to resend a KA probe if previous
		                 probe was unacked.                                  */
		KeepAliveOptions();
	} KeepAliveOptions;

	typedef enum class GroupApplication {
		JOIN  = IP_ADD_MEMBERSHIP,
		LEAVE = IP_DROP_MEMBERSHIP
	} GroupApplication;


	typedef enum class SocketChannel {
	#if defined(__WINDOWS__)
		BOTH  = SD_BOTH,
		READ  = SD_RECEIVE,
		WRITE = SD_SEND,
	#else
		BOTH  = SHUT_RDWR,
		READ  = SHUT_RD,
		WRITE = SHUT_WR,
	#endif
	} SocketChannel;


	typedef enum class SocketDomain {
		// !- CROSS-PLATFORM DOMAINS    -!
		UNSPECIFIED = AF_UNSPEC,   // Address is unspecified
		INET        = AF_INET,     // IPv4 Internet protocols
		INET6       = AF_INET6,    // IPv6 Internet protocols

		// !- PLATFORM-SPECIFIC DOMAINS -!
	#if defined(__WINDOWS__)
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
	#if defined(__WINDOWS__)
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


	typedef enum class MessageFlags {
		// !- CROSS-PLATFORM FLAGS    -!
		NONE = 0,

		DONTROUTE = MSG_DONTROUTE, /* Don't use a gateway to send out the
		packet, send to hosts only on directly connected networks.  This is
		usually used only by diagnostic or routing programs.  This is
		defined only for protocol families that route; packet sockets don't.*/

		OOB = MSG_OOB, /* Sends out-of-band data on sockets that support
		this notion (e.g., of type SOCK_STREAM); the underlying protocol
		must also support out-of-band data.*/

		PEEK    = MSG_PEEK,    /* Peek at incoming message. */
		WAITALL = MSG_WAITALL, /* Wait for full request or error. */

		// !- PLATFORM-SPECIFIC FLAGS -!
	#if defined(__WINDOWS__)
		PUSH_IMMEDIATE = MSG_PUSH_IMMEDIATE, /* Do not delay receive request completion if data is available. */
		PARTIAL        = MSG_PARTIAL,        /* Partial send or recv for message xport. */
		INTERRUPT      = MSG_INTERRUPT,      /* send/recv in the interrupt context. */
	#elif defined(__APPLE__)
		EOR            = MSG_EOR,            /* Data completes record. */
		TRUNC          = MSG_TRUNC,          /* Data discarded before delivery. */
		CTRUNC         = MSG_CTRUNC,         /* Control data lost before delivery. */
		DONTWAIT       = MSG_DONTWAIT,       /* This message should be nonblocking. */
		EOF            = MSG_EOF,            /* Data completes connection. */
		FLUSH          = MSG_FLUSH,          /* Start of 'hold' sequence; dump so_temp. */
		HOLD           = MSG_HOLD,           /* Hold fragment in so_temp. */
		SEND           = MSG_SEND,           /* Send the packet in so_temp. */
		HAVEMORE       = MSG_HAVEMORE,       /* Data ready to be read. */
		RCVMORE        = MSG_RCVMORE,        /* Data remains in current packet. */
		NEEDSA         = MSG_NEEDSA,         /* Fail receive if socket address cannot be allocated. */
	#else /* linux */
		CTRUNC         = MSG_CTRUNC,         /* Control data lost before delivery. */
		PROXY          = MSG_PROXY,          /* Supply or ask second address. */
		TRUNC          = MSG_TRUNC,          /* Data discarded before delivery. */
		FIN            = MSG_FIN,
		SYN            = MSG_SYN,
		RST            = MSG_RST,
		ERRQUEUE       = MSG_ERRQUEUE,       /* Fetch message from error queue. */
		WAITFORONE     = MSG_WAITFORONE,     /* Wait for at least one packet to return. */
		FASTOPEN       = MSG_FASTOPEN,       /* Send data in TCP SYN. */

		#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,4)
		MORE = MSG_MORE, /* (since Linux 2.4.4) The caller has more data
		to send.  This flag is used with TCP sockets to obtain the same
		effect as the TCP_CORK socket option (see tcp(7)), with the
		difference that this flag can be set on a per-call basis.

		Since Linux 2.6, this flag is also supported for UDP sockets, and
		informs the kernel to package all of the data sent in calls with
		this flag set into a single datagram which is transmitted only
		when a call is performed that does not specify this flag.  (See
		also the UDP_CORK socket option described in udp(7).) */
		#endif

		#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,15)
		CONFIRM = MSG_CONFIRM, /* (since Linux 2.3.15) Tell the link layer
		that forward progress happened: you got a successful reply from the
		other side.  If the link layer doesn't get this it will regularly
		reprobe the neighbor (e.g., via a unicast ARP).  Valid only on
		SOCK_DGRAM and SOCK_RAW sockets and currently implemented only for
		IPv4 and IPv6.  See arp(7) for details. */
		#endif

		#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
		DONTWAIT = MSG_DONTWAIT, /* (since Linux 2.2) Enables nonblocking
		operation; if the operation would block, EAGAIN or EWOULDBLOCK is
		returned.  This provides similar behavior to setting the O_NONBLOCK
		flag (via the fcntl(2) F_SETFL operation), but differs in that
		MSG_DONTWAIT is a per-call option, whereas O_NONBLOCK is a setting
		on the open file description (see open(2)), which will affect all
		threads in the calling process and as well as other processes that
		hold file descriptors referring to the same open file description.*/

		EOR = MSG_EOR, /* (since Linux 2.2) Terminates a record (when this
		notion is supported, as for sockets of type SOCK_SEQPACKET). */

		NOSIGNAL = MSG_NOSIGNAL, /* (since Linux 2.2) Don't generate a
		SIGPIPE signal if the peer on a stream-oriented socket has closed
		the connection.  The EPIPE error is still returned.  This provides
		similar behavior to using sigaction(2) to ignore SIGPIPE, but,
		whereas MSG_NOSIGNAL is a per-call feature, ignoring SIGPIPE sets
		a process attribute that affects all threads in the process. */
		#endif
	#endif
	} MessageFlags;
	ENUM_OPERATOR(MessageFlags, int, |)


	typedef enum class PollFlags {
		// !- CROSS-PLATFORM FLAGS    -!
		IN          = POLLIN,    /* There is data to read. */
		OUT         = POLLOUT,   /* Writing now will not block. */
		PRIORITY_IN = POLLPRI,   /* There is urgent data to read. */
		ERROR       = POLLERR,   /* Error condition. */
		HANGUP      = POLLHUP,   /* Peer hung up. */
		INVALID     = POLLNVAL,  /* Invalid polling request. */

		// !- PLATFORM-SPECIFIC FLAGS -!
	#if defined(__APPLE__)
		// FreeBSD
		EXTEND      = POLLEXTEND, /* File may have been extended. */
		ATTRIBUTE   = POLLATTRIB, /* File attributes may have changed. */
		NEW_LINK    = POLLNLINK,  /* (Un)link/rename may have happened. */
		WRITE       = POLLWRITE,  /* File's contents may have changed. */
	#elif defined (__LINUX__)
		#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,17)
		RDHUP       = POLLRDHUP, /* Peer closed connection, or shut down writing half of connection. */
		#endif
		MESSAGE     = POLLMSG,
		REMOVE      = POLLREMOVE,
		READ_HANGUP = POLLRDHUP,
	#endif
	} PollFlags;
	ENUM_OPERATOR(PollFlags, int, |)
	ENUM_OPERATOR(PollFlags, int, &)


#if defined(__WINDOWS__)
	#define SERROR(x) WSA##x
#else
	#define SERROR(x) x
#endif
	typedef enum class SocketError { /* errno | WSAGetLastError */
		// !- CROSS-PLATFORM FLAGS    -!
		OTHER                        = -1,
		SUCCESS                      = 0,

		ACCESS                       = SERROR(EACCES),
		ADDRESS_FAMILY_NOT_SUPPORTED = SERROR(EAFNOSUPPORT),
		ADDRESS_IN_USE               = SERROR(EADDRINUSE),
		ADDRESS_NOT_AVAILABLE        = SERROR(EADDRNOTAVAIL),
		ALREADY_IN_PROGRESS          = SERROR(EALREADY),
		BAD_DESCRIPTOR               = SERROR(EBADF),
		BAD_PROTOCOL_OPTION          = SERROR(ENOPROTOOPT),
		CONNECTION_ABORTED           = SERROR(ECONNABORTED),
		CONNECTION_REFUSED           = SERROR(ECONNREFUSED),
		CONNECTION_RESET             = SERROR(ECONNRESET),
		DESTINATION_ADDRESS_REQUIRED = SERROR(EDESTADDRREQ),
		DISC_QUOTA_EXCEEDED          = SERROR(EDQUOT),
		FAULT                        = SERROR(EFAULT),
		HOST_UNREACHABLE             = SERROR(EHOSTUNREACH),
		IN_PROGRESS                  = SERROR(EINPROGRESS),
		INTERRUPTED                  = SERROR(EINTR),
		INVALID                      = SERROR(EINVAL),
		IS_CONNECTED                 = SERROR(EISCONN),
		LOOP                         = SERROR(ELOOP),
		MESSAGE_SIZE                 = SERROR(EMSGSIZE),
		NAME_TOO_LONG                = SERROR(ENAMETOOLONG),
		NETWORK_DOWN                 = SERROR(ENETDOWN),
		NETWORK_UNREACHABLE          = SERROR(ENETUNREACH),
		NO_BUFFER_SPACE              = SERROR(ENOBUFS),
		NO_MORE_DESCRIPTORS          = SERROR(EMFILE),
		NOT_CONNECTED                = SERROR(ENOTCONN),
		NOT_SOCKET                   = SERROR(ENOTSOCK),
		OPERATION_NOT_SUPPORTED      = SERROR(EOPNOTSUPP),
		PROTOCOL_NOT_SUPPORTED       = SERROR(EPROTONOSUPPORT),
		TIMED_OUT                    = SERROR(ETIMEDOUT),
		WOULD_BLOCK                  = SERROR(EWOULDBLOCK),
		WRONG_PROTOTYPE              = SERROR(EPROTOTYPE),

		// !- PLATFORM-SPECIFIC FLAGS -!
	#if defined(__WINDOWS__)
		INVALID_PROVIDER             = SERROR(EINVALIDPROVIDER),
		INVALID_PROCEDURE_TABLE      = SERROR(EINVALIDPROCTABLE),
		NOT_INITIALIZED              = SERROR(NOTINITIALISED),
		PROVIDER_INIT_FAILED         = SERROR(EPROVIDERFAILEDINIT),
		SHUTDOWN                     = SERROR(ESHUTDOWN),
		TYPE_NOT_SUPPORTED           = SERROR(ESOCKTNOSUPPORT),
	#else /* Linux | OSX */
		AGAIN                        = SERROR(EAGAIN), /* Recommended: WOULD_BLOCK */
		DOMAIN_ERROR                 = SERROR(EDOM),
		IO_ERROR                     = SERROR(EIO),
		IS_DIRECTORY                 = SERROR(EISDIR), /* Recommended: DESTINATION_ADDRESS_REQUIRED */
		NO_ENTRY                     = SERROR(ENOENT),
		NO_MEMORY                    = SERROR(ENOMEM),
		NO_SPACE                     = SERROR(ENOSPC), /* Recommended: DISC_QUOTA_EXCEEDED */
		NOT_DIRECTORY                = SERROR(ENOTDIR),
		PERMISSION                   = SERROR(EPERM),
		PIPE_ERROR                   = SERROR(EPIPE),
		PROTOCOL                     = SERROR(EPROTO),
		READ_ONLY_FILESYSTEM         = SERROR(EROFS),
		SYSTEM_DLIMIT_REACHED        = SERROR(ENFILE),
	#endif
	} SocketError;


	typedef enum class HostSocketError { /* h_errno | WSAGetLastError */
		// !- CROSS-PLATFORM FLAGS    -!
		NOTFOUND   = SERROR(HOST_NOT_FOUND),
		NODATA     = SERROR(NO_DATA),
		NORECOVERY = SERROR(NO_RECOVERY),
		TRYAGAIN   = SERROR(TRY_AGAIN),
	} HostSocketError;
#undef ERROR
}

#if defined(__clang__)
  #pragma pop_macro("EOF");
#endif
#undef ENUM_OPERATOR
#endif
