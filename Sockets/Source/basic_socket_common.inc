/**
 * Created by TekuConcept on July 11, 2018
 */

#include <stdlib.h>            // For atoi
#include <errno.h>             // For errno
#include <cstring>             // For strerror and memset
#include <sstream>
#include <stdexcept>           // For runtime_error
#include <memory>

#include "utils/environment.h"
#include "utils/impact_error.h"
#include "sockets/generic.h"

#include <sys/types.h>         // For data types

#if defined(__OS_WINDOWS__)
    #pragma pop_macro("IN")    // pushed in SocketTypes.h
    #pragma pop_macro("OUT")   // pushed in SocketTypes.h
    #pragma pop_macro("ERROR") // pushed in SocketTypes.h
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <mstcpip.h>       // struct tcp_keepalive
    #include <iphlpapi.h>
    #include <Ws2def.h>        // reuse address
#else
    #include <sys/socket.h>    // For socket(), connect(), send(), and recv()
    #include <sys/ioctl.h>     // For ioctl()
    #include <sys/poll.h>      // For struct pollfd, poll()
    #include <net/if.h>        // For ifconf
    #include <netdb.h>         // For gethostbyname()
    #include <netinet/in.h>    // For sockaddr_in
    #include <netinet/tcp.h>   // For IPPROTO_TCP, TCP_KEEPCNT, TCP_KEEPINTVL,
                               // TCP_KEEPIDLE
    #include <arpa/inet.h>     // For inet_addr(), ntohs()
    #include <unistd.h>        // For close()
    #include <ifaddrs.h>       // getifaddrs(), freeifaddrs()
#if defined(__OS_APPLE__)
    #include <net/if_types.h>  // For IFT_XXX types
    #include <net/if_dl.h>     // For sockaddr_dl
#elif defined(__OS_LINUX__)
    #include <net/if_arp.h>
#endif
#endif

#if defined(__OS_WINDOWS__)
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

#define ASSERT(cond)\
    if (!(cond)) throw impact_error(internal::error_message());

#define ASSERT_MOVED\
    if (!m_info_) throw impact_error("Socket moved");

#define ASSERT_INVALID\
    if (m_info_->descriptor == INVALID_SOCKET)\
        throw impact_error("Invalid socket");

#define CATCH_ASSERT(code)\
    try { code }\
    catch (impact_error&) { throw; }\
    catch (...) { throw impact_error("Unknown internal error"); }

#define WIN_ASSERT(cond,error,fin)\
    if (!(cond)) {\
        fin\
        throw impact_error(internal::win_error_message( error ));\
    }
