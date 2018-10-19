/**
 * Created by TekuConcept on July 7, 2018
 */

#include "sockets/generic.h"

#include <sstream>            // ostringstream
#include <cstring>            // strerror, memset
#include <csignal>            // sigaction
#include <atomic>             // atomic

#include "utils/impact_error.h"

#if defined(__OS_LINUX__)
    #include <netdb.h>          // h_errno
#elif defined(__OS_WINDOWS__)
    #pragma pop_macro("IN")     // pushed in SocketTypes.h
    #pragma pop_macro("OUT")    // pushed in SocketTypes.h
    #pragma pop_macro("ERROR")  // pushed in SocketTypes.h
    #include <ws2tcpip.h>
    #pragma comment (lib, "Ws2_32.lib")
    #undef ASSERT
#endif

#define ASSERT(cond)\
    if (!(cond)) throw impact_error(internal::error_message());


using namespace impact;

int
internal::error_code()
{
#if defined(__OS_WINDOWS__)
    return WSAGetLastError();
#else
    return errno;
#endif
}


std::string
internal::error_message()
{
    std::ostringstream os;
#if defined(__OS_WINDOWS__)
    auto error_code = WSAGetLastError();

    std::string data;
    auto status = wsa_error_string(error_code, data);
    if (status == -1) {
        data.resize(128, '\0');
        status = FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error_code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            &data[0],
            data.size(),
            NULL
        );
        if (status == 0)
            os << "[No Error Message Available (" << error_code << ")]";
        else os << data;
    }
    else os << data;
#else
    os << strerror(errno);
    os << " [" << errno << "]";
#endif
    return os.str();
}


// TODO: Support IPv6
void
internal::fill_address(
    socket_domain                     __domain,
    socket_type                       __type,
    socket_protocol                   __protocol,
    const std::string&                __address,
    const unsigned short              __port,
    std::shared_ptr<struct sockaddr>* __socket_address_info)
{
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family   = (int)__domain;
    hints.ai_socktype = (int)__type;
    hints.ai_protocol = (int)__protocol;
    hints.ai_flags    = AI_PASSIVE;

    auto port   = std::to_string(__port);
    auto status = ::getaddrinfo(&__address[0], &port[0], &hints, &result);

#if !defined(__OS_WINDOWS__)
    ASSERT(status != EAI_SYSTEM);
#endif
    if (status != 0)
        throw impact_error(::gai_strerror(status));

    struct sockaddr* copy;
    if (__domain == socket_domain::INET) {
        copy = (struct sockaddr*)(new struct sockaddr_in);
        memcpy(copy, result->ai_addr, sizeof(struct sockaddr_in));
    }
    else if (__domain == socket_domain::INET6) {
        copy = (struct sockaddr*)(new struct sockaddr_in6);
        memcpy(copy, result->ai_addr, sizeof(struct sockaddr_in6));
    }
    else throw impact_error("Unsupported domain");

    *__socket_address_info = std::shared_ptr<struct sockaddr>(copy);
    
    freeaddrinfo(result);
}


void
internal::fill_address(
    const std::vector<unsigned char>& __address,
    const unsigned short              __port,
    std::shared_ptr<struct sockaddr>* __socket_address_info)
{
    struct sockaddr* copy;
    if (__address.size() == IPV4_ADDRESS_SIZE) {
        copy = (struct sockaddr*)(new struct sockaddr_in);
        struct sockaddr_in& socket_address = *(struct sockaddr_in*)copy;
        memset(&socket_address, 0, sizeof(socket_address));
        socket_address.sin_family = AF_INET;
        socket_address.sin_port = __port;
        socket_address.sin_addr.S_un.S_un_b.s_b1 = __address[0];
        socket_address.sin_addr.S_un.S_un_b.s_b2 = __address[1];
        socket_address.sin_addr.S_un.S_un_b.s_b3 = __address[2];
        socket_address.sin_addr.S_un.S_un_b.s_b4 = __address[3];
    }
    else if (__address.size() == IPV6_ADDRESS_SIZE) {
        copy = (struct sockaddr*)(new struct sockaddr_in6);
        struct sockaddr_in6& socket_address = *(struct sockaddr_in6*)copy;
        memset(&socket_address, 0, sizeof(socket_address));
        socket_address.sin6_family = AF_INET6;
        socket_address.sin6_port = __port;
        for (int i = 0; i < IPV6_ADDRESS_SIZE; i++)
            socket_address.sin6_addr.u.Byte[i] = __address[i];
    }
    else throw impact_error("Unsupported address size");

    *__socket_address_info = std::shared_ptr<struct sockaddr>(copy);
}


std::vector<unsigned char>
internal::sock_addr_byte_array(const struct sockaddr* __address)
{
    std::vector<unsigned char> result;

    if (__address->sa_family == AF_INET) {
        result.resize(IPV4_ADDRESS_SIZE);
        struct sockaddr_in& socket_address =
            *(struct sockaddr_in*)(__address);
        result[0] = socket_address.sin_addr.S_un.S_un_b.s_b1;
        result[1] = socket_address.sin_addr.S_un.S_un_b.s_b2;
        result[2] = socket_address.sin_addr.S_un.S_un_b.s_b3;
        result[3] = socket_address.sin_addr.S_un.S_un_b.s_b4;
    }
    else if (__address->sa_family == AF_INET6) {
        result.resize(IPV6_ADDRESS_SIZE);
        struct sockaddr_in6& socket_address =
            *(struct sockaddr_in6*)(__address);
        for (int i = 0; i < IPV6_ADDRESS_SIZE; i++)
            result[i] = socket_address.sin6_addr.u.Byte[i];
    }
    else throw impact_error("Unsupported address family");

    return result;
}


std::string
internal::sock_addr_string(const struct sockaddr* __address)
{
    if (!__address)
        return "";

    switch (__address->sa_family) {
    case AF_INET: {
        char buffer[INET_ADDRSTRLEN];
        struct sockaddr_in* socket_address = (struct sockaddr_in*)__address;
        auto result = inet_ntop(
            AF_INET,
            &socket_address->sin_addr,
            buffer,
            INET_ADDRSTRLEN
        );
        if (result == NULL)
            return "";
        else return std::string(result);
    }

    case AF_INET6: {
        char buffer[INET6_ADDRSTRLEN];
        struct sockaddr_in6* socket_address = (struct sockaddr_in6*)__address;
        auto result = inet_ntop(
            AF_INET6,
            &socket_address->sin6_addr,
            buffer,
            INET6_ADDRSTRLEN
        );
        if (result == NULL)
            return "";
        else return std::string(result);
    }

    default: return "";
    };
}


#if defined(__OS_WINDOWS__)
std::string
internal::to_narrow_string(
    const wchar_t*     __original,
    char               __unknown,
    const std::locale& __locale)
{
    std::ostringstream os;
    while (*__original != L'\0')
        os << std::use_facet< std::ctype<wchar_t> >(__locale)
        .narrow(*__original++, __unknown);
    return os.str();
}


std::string
internal::win_error_message(unsigned long __code)
{
    switch (__code) {
    case WSASYSNOTREADY:
        return "The underlying network subsystem is not ready for network "
            "communication.";
    case WSAVERNOTSUPPORTED:
        return "The version of Windows Sockets support requested is not provided by"
            " this particular Windows Sockets implementation.";
    case WSAEINPROGRESS:
        return "A blocking Windows Sockets 1.1 operation is in progress.";
    case WSAEPROCLIM:
        return "A limit on the number of tasks supported by the Windows Sockets "
            "implementation has been reached.";
    case WSAEFAULT:
        return "The lpWSAData parameter is not a valid pointer.";
    case ERROR_ADDRESS_NOT_ASSOCIATED:
        return "An address has not yet been associated with the network endpoint. "
            "DHCP lease information was available.";
    case ERROR_BUFFER_OVERFLOW:
        return "The buffer size indicated by the SizePointer parameter is too small"
            " to hold the adapter information or the AdapterAddresses parameter is NULL"
            ". The SizePointer parameter returned points to the required size of the "
            "buffer to hold the adapter information.";
    case ERROR_INVALID_PARAMETER:
        return "One of the parameters is invalid.This error is returned for any of "
            "the following conditions : the SizePointer parameter is NULL, the Address "
            "parameter is not AF_INET, AF_INET6, or AF_UNSPEC, or the address "
            "information for the parameters requested is greater than ULONG_MAX.";
    case ERROR_NOT_ENOUGH_MEMORY:
        return "Insufficient memory resources are available to complete the "
            "operation.";
    case ERROR_NO_DATA:
        return "No addresses were found for the requested parameters.";
    default:
        std::string data(128, '\0');

        auto status = FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            __code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            &data[0],
            128,
            NULL
        );

        if (status == 0)
            return "[No Error Message Available]";
        else return data;
    }
}


int
internal::wsa_error_string(
    unsigned long __code,
    std::string&  __buffer)
{
#ifdef IMPACT_NO_WSAESTR
    UNUSED(__code);
    UNUSED(__buffer);
    return -1;
#else
    switch (__code) {
        /* WSA_INVALID_HANDLE */
    case 6: __buffer = "Specified event object handle is invalid."; break;
        /* WSA_NOT_ENOUGH_MEMORY */
    case 8: __buffer = "Insufficient memory available."; break;
        /* WSA_INVALID_PARAMETER */
    case 87: __buffer = "One or more parameters are invalid."; break;
        /* WSA_OPERATION_ABORTED */
    case 995: __buffer = "Overlapped operation aborted."; break;
        /* WSA_IO_INCOMPLETE */
    case 996: __buffer = "Overlapped I/O event object not in signaled state."; break;
        /* WSA_IO_PENDING */
    case 997: __buffer = "Overlapped operations will complete later. "; break;
        /* WSAEINTR */
    case 10004: __buffer = "Interrupted function call."; break;
        /* WSAEBADF */
    case 10009: __buffer = "File handle is not valid."; break;
        /* WSAEACCES */
    case 10013: __buffer = "Permission denied."; break;
        /* WSAEFAULT */
    case 10014: __buffer = "Bad address."; break;
        /* WSAEINVAL */
    case 10022: __buffer = "Invalid argument."; break;
        /* WSAEMFILE */
    case 10024: __buffer = "Too many open files."; break;
        /* WSAEWOULDBLOCK */
    case 10035: __buffer = "Resource temporarily unavailable."; break;
        /* WSAEINPROGRESS */
    case 10036: __buffer = "Operation now in progress."; break;
        /* WSAEALREADY */
    case 10037: __buffer = "Operation already in progress."; break;
        /* WSAENOTSOCK */
    case 10038: __buffer = "Socket operation on nonsocket."; break;
        /* WSAEDESTADDRREQ */
    case 10039: __buffer = "Destination address required."; break;
        /* WSAEMSGSIZE */
    case 10040: __buffer = "Message too long."; break;
        /* WSAEPROTOTYPE */
    case 10041: __buffer = "Protocol wrong type for socket."; break;
        /* WSAENOPROTOOPT */
    case 10042: __buffer = "Bad protocol option."; break;
        /* WSAEPROTONOSUPPORT */
    case 10043: __buffer = "Protocol not supported."; break;
        /* WSAESOCKTNOSUPPORT */
    case 10044: __buffer = "Socket type not supported."; break;
        /* WSAEOPNOTSUPP */
    case 10045: __buffer = "Operation not supported."; break;
        /* WSAEPFNOSUPPORT */
    case 10046: __buffer = "Protocol family not supported."; break;
        /* WSAEAFNOSUPPORT */
    case 10047: __buffer = "Address family not supported by protocol family."; break;
        /* WSAEADDRINUSE */
    case 10048: __buffer = "Address already in use."; break;
        /* WSAEADDRNOTAVAIL */
    case 10049: __buffer = "Cannot assign requested address."; break;
        /* WSAENETDOWN */
    case 10050: __buffer = "Network is down."; break;
        /* WSAENETUNREACH */
    case 10051: __buffer = "Network is unreachable."; break;
        /* WSAENETRESET */
    case 10052: __buffer = "Network dropped connection on reset."; break;
        /* WSAECONNABORTED */
    case 10053: __buffer = "Software caused connection abort."; break;
        /* WSAECONNRESET */
    case 10054: __buffer = "Connection reset by peer."; break;
        /* WSAENOBUFS */
    case 10055: __buffer = "No buffer space available."; break;
        /* WSAEISCONN */
    case 10056: __buffer = "Socket is already connected."; break;
        /* WSAENOTCONN */
    case 10057: __buffer = "Socket is not connected."; break;
        /* WSAESHUTDOWN */
    case 10058: __buffer = "Cannot send after socket shutdown."; break;
        /* WSAETOOMANYREFS */
    case 10059: __buffer = "Too many references."; break;
        /* WSAETIMEDOUT */
    case 10060: __buffer = "Connection timed out."; break;
        /* WSAECONNREFUSED */
    case 10061: __buffer = "Connection refused."; break;
        /* WSAELOOP */
    case 10062: __buffer = "Cannot translate name."; break;
        /* WSAENAMETOOLONG */
    case 10063: __buffer = "Name too long."; break;
        /* WSAEHOSTDOWN */
    case 10064: __buffer = "Host is down."; break;
        /* WSAEHOSTUNREACH */
    case 10065: __buffer = "No route to host."; break;
        /* WSAENOTEMPTY */
    case 10066: __buffer = "Directory not empty."; break;
        /* WSAEPROCLIM */
    case 10067: __buffer = "Too many processes."; break;
        /* WSAEUSERS */
    case 10068: __buffer = "User quota exceeded."; break;
        /* WSAEDQUOT */
    case 10069: __buffer = "Disk quota exceeded."; break;
        /* WSAESTALE */
    case 10070: __buffer = "Stale file handle reference."; break;
        /* WSAEREMOTE */
    case 10071: __buffer = "Item is remote."; break;
        /* WSASYSNOTREADY */
    case 10091: __buffer = "Network subsystem is unavailable."; break;
        /* WSAVERNOTSUPPORTED */
    case 10092: __buffer = "Winsock.dll version out of range."; break;
        /* WSANOTINITIALISED */
    case 10093: __buffer = "Successful WSAStartup not yet performed."; break;
        /* WSAEDISCON */
    case 10101: __buffer = "Graceful shutdown in progress."; break;
        /* WSAENOMORE */
    case 10102: __buffer = "No more results."; break;
        /* WSAECANCELLED */
    case 10103: __buffer = "Call has been canceled."; break;
        /* WSAEINVALIDPROCTABLE */
    case 10104: __buffer = "Procedure call table is invalid."; break;
        /* WSAEINVALIDPROVIDER */
    case 10105: __buffer = "Service provider is invalid."; break;
        /* WSAEPROVIDERFAILEDINIT */
    case 10106: __buffer = "Service provider failed to initialize."; break;
        /* WSASYSCALLFAILURE */
    case 10107: __buffer = "System call failure."; break;
        /* WSASERVICE_NOT_FOUND */
    case 10108: __buffer = "Service not found."; break;
        /* WSATYPE_NOT_FOUND */
    case 10109: __buffer = "Class type not found."; break;
        /* WSA_E_NO_MORE */
    case 10110: __buffer = "No more results."; break;
        /* WSA_E_CANCELLED */
    case 10111: __buffer = "Call was canceled."; break;
        /* WSAEREFUSED */
    case 10112: __buffer = "Database query was refused."; break;
        /* WSAHOST_NOT_FOUND */
    case 11001: __buffer = "Host not found."; break;
        /* WSATRY_AGAIN */
    case 11002: __buffer = "Nonauthoritative host not found."; break;
        /* WSANO_RECOVERY */
    case 11003: __buffer = "This is a nonrecoverable error."; break;
        /* WSANO_DATA */
    case 11004: __buffer = "Valid name, no data record of requested type."; break;
        /* WSA_QOS_RECEIVERS */
    case 11005: __buffer = "QoS receivers."; break;
        /* WSA_QOS_SENDERS */
    case 11006: __buffer = "QoS senders."; break;
        /* WSA_QOS_NO_SENDERS */
    case 11007: __buffer = "No QoS senders."; break;
        /* WSA_QOS_NO_RECEIVERS */
    case 11008: __buffer = "QoS no receivers."; break;
        /* WSA_QOS_REQUEST_CONFIRMED */
    case 11009: __buffer = "QoS request confirmed."; break;
        /* WSA_QOS_ADMISSION_FAILURE */
    case 11010: __buffer = "QoS admission error."; break;
        /* WSA_QOS_POLICY_FAILURE */
    case 11011: __buffer = "QoS policy failure."; break;
        /* WSA_QOS_BAD_STYLE */
    case 11012: __buffer = "QoS bad style."; break;
        /* WSA_QOS_BAD_OBJECT */
    case 11013: __buffer = "QoS bad object."; break;
        /* WSA_QOS_TRAFFIC_CTRL_ERROR */
    case 11014: __buffer = "QoS traffic control error."; break;
        /* WSA_QOS_GENERIC_ERROR */
    case 11015: __buffer = "QoS generic error."; break;
        /* WSA_QOS_ESERVICETYPE */
    case 11016: __buffer = "QoS service type error."; break;
        /* WSA_QOS_EFLOWSPEC */
    case 11017: __buffer = "QoS flowspec error."; break;
        /* WSA_QOS_EPROVSPECBUF */
    case 11018: __buffer = "Invalid QoS provider buffer."; break;
        /* WSA_QOS_EFILTERSTYLE */
    case 11019: __buffer = "Invalid QoS filter style."; break;
        /* WSA_QOS_EFILTERTYPE */
    case 11020: __buffer = "Invalid QoS filter type."; break;
        /* WSA_QOS_EFILTERCOUNT */
    case 11021: __buffer = "Incorrect QoS filter count."; break;
        /* WSA_QOS_EOBJLENGTH */
    case 11022: __buffer = "Invalid QoS object length."; break;
        /* WSA_QOS_EFLOWCOUNT */
    case 11023: __buffer = "Incorrect QoS flow count."; break;
        /* WSA_QOS_EUNKOWNPSOBJ */
    case 11024: __buffer = "Unrecognized QoS object."; break;
        /* WSA_QOS_EPOLICYOBJ */
    case 11025: __buffer = "Invalid QoS policy object."; break;
        /* WSA_QOS_EFLOWDESC */
    case 11026: __buffer = "Invalid QoS flow descriptor."; break;
        /* WSA_QOS_EPSFLOWSPEC */
    case 11027: __buffer = "Invalid QoS provider-specific flowspec."; break;
        /* WSA_QOS_EPSFILTERSPEC */
    case 11028: __buffer = "Invalid QoS provider-specific filterspec."; break;
        /* WSA_QOS_ESDMODEOBJ */
    case 11029: __buffer = "Invalid QoS shape discard mode object."; break;
        /* WSA_QOS_ESHAPERATEOBJ */
    case 11030: __buffer = "Invalid QoS shaping rate object."; break;
        /* WSA_QOS_RESERVED_PETYPE */
    case 11031: __buffer = "Reserved policy QoS element type."; break;
    default: return -1;
    }
    return 0;
#endif
}
#else

namespace impact {
namespace internal {
    // only attempt to disable sigpipe once
    std::atomic<bool> _s_sigpipe_blocked_(false);
}}


void
internal::no_sigpipe()
{
    if (!_s_sigpipe_blocked_) {
        struct sigaction action;
        std::memset(&action, 0, sizeof(action));
        action.sa_handler = SIG_IGN;
        action.sa_flags   = SA_RESTART;
        auto status = ::sigaction(SIGPIPE, &action, NULL);
        // EFAULT on 'act' and 'oldact' should never happen
        // EINVAL should never happen since SIGPIPE is a valid signal
        // however, some systems might not consider the signal as valid
        ASSERT(status != -1)
        _s_sigpipe_blocked_ = true;
    }
}
#endif
