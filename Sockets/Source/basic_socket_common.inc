/**
 * Created by TekuConcept on July 11, 2018
 */

#include <sys/types.h>			 // For data types
#include <stdlib.h>				   // For atoi
#include <errno.h>				   // For errno
#include <cstring>				   // For strerror and memset
#include <sstream>
#include <stdexcept>         // For runtime_error

#include "sockets/environment.h"
#include "sockets/generic.h"
#include "sockets/io_error.h"

#if defined(__WINDOWS__)
	#pragma pop_macro("IN")    // pushed in SocketTypes.h
	#pragma pop_macro("OUT")   // pushed in SocketTypes.h
	#pragma pop_macro("ERROR") // pushed in SocketTypes.h
	#include <winsock2.h>
	#include <ws2tcpip.h>
 	#include <mstcpip.h>       // struct tcp_keepalive
	#include <iphlpapi.h>
	#include <Ws2def.h>        // reuse address
#else
	#include <sys/socket.h>		 // For socket(), connect(), send(), and recv()
	#include <sys/ioctl.h>     // For ioctl()
  #include <sys/poll.h>      // For struct pollfd, poll()
	#include <net/if.h>        // For ifconf
	#include <netdb.h>			   // For gethostbyname()
  #include <netinet/in.h>    // For sockaddr_in
 	#include <netinet/tcp.h>	 // For IPPROTO_TCP, TCP_KEEPCNT, TCP_KEEPINTVL,
 								             // TCP_KEEPIDLE
	#include <arpa/inet.h>		 // For inet_addr(), ntohs()
	#include <unistd.h>			   // For close()
  #include <ifaddrs.h>       // getifaddrs(), freeifaddrs()
#if defined(__APPLE__)
	#include <net/if_types.h>  // For IFT_XXX types
	#include <net/if_dl.h>     // For sockaddr_dl
#elif defined(__LINUX__)
	#include <net/if_arp.h>
#endif
#endif

#if defined(__WINDOWS__)
	#define CLOSE_SOCKET(x) closesocket(x)
	#define SOC_POLL WSAPoll
	#define CCHAR_PTR const char*
	#define CHAR_PTR char*
	#undef ASSERT
	#pragma warning(disable:4996)
	#pragma comment (lib, "Ws2_32.lib")
	#pragma comment (lib, "Mswsock.lib")
	#pragma comment (lib, "AdvApi32.lib")
	#pragma comment (lib, "IPHLPAPI.lib")
#else
	#define CLOSE_SOCKET(x) ::close(x)
	#define SOC_POLL ::poll
	#define CCHAR_PTR void*
	#define CHAR_PTR void*
 	#define SOCKET_ERROR -1
 	#define INVALID_SOCKET -1
#endif

#define CATCH_ASSERT(title,code)\
 	try { code }\
 	catch (std::runtime_error e) {\
 		std::string message( title );\
 		message.append(e.what());\
 		throw io_error(message);\
 	}

#define ASSERT(title,cond)\
 	if (cond) {\
 		std::string message( title );\
 		message.append(internal::error_message());\
 		throw io_error(message);\
 	}

#define ASSERT_MOVED(title)\
	if (!m_info_) throw std::runtime_error(title "Socket moved");

#define WIN_ASSERT(title,cond,error,fin)\
	if (cond) {\
		fin\
		std::string message( title );\
		message.append(internal::win_error_message( error ));\
		throw io_error(message);\
	}
