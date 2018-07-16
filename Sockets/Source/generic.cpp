/**
 * Created by TekuConcept on July 7, 2018
 */

#include "sockets/generic.h"

#include <sstream>            // ostringstream
#include <cstring>            // strerror, memset
#include <stdexcept>          // runtime_error
#include <csignal>            // sigaction
#include <atomic>             // atomic

#if defined(__LINUX__)
  #include <netdb.h>          // h_errno
#elif defined(__WINDOWS__)
	#pragma pop_macro("IN")     // pushed in SocketTypes.h
	#pragma pop_macro("OUT")    // pushed in SocketTypes.h
	#pragma pop_macro("ERROR")  // pushed in SocketTypes.h
	#include <ws2tcpip.h>
	#pragma comment (lib, "Ws2_32.lib")
	#undef ASSERT
#endif

#define ASSERT(title,cond)\
 	if (cond) {\
 		std::string message( title );\
 		message.append(internal::error_message());\
 		throw std::runtime_error(message);\
 	}


using namespace impact;


#if defined(__WINDOWS__)
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
	    ASSERT("internal::no_sigpipe()\n", status == -1);
		_s_sigpipe_blocked_ = true;
	}
}
#endif


std::string
internal::error_message()
{
	std::ostringstream os;
#if defined(__WINDOWS__)
	char data[128];
	auto error_code = WSAGetLastError();

  auto status = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error_code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		data,
		128,
		NULL
	);

	if(status == 0) os << "[No Error Message Available]";
	else os << data;
#else
	os << strerror(errno);
#endif
	return os.str();
}


// TODO: Support IPv6
void
internal::fill_address(
	socket_domain        __domain,
	socket_type          __type,
	socket_protocol      __protocol,
	const std::string&   __address,
	const unsigned short __port,
	struct sockaddr_in&  __socket_address)
{
	struct addrinfo hints, *result;
	memset(&hints, 0, sizeof(hints));
	memset(&__socket_address, 0, sizeof(__socket_address));

	UNUSED(__domain);
	hints.ai_family   = AF_INET;//(int)__domain;
	hints.ai_socktype = (int)__type;
	hints.ai_flags    = AI_PASSIVE;
	hints.ai_protocol = (int)__protocol;

	auto port   = std::to_string(__port);
	auto status = ::getaddrinfo(&__address[0], &port[0], &hints, &result);
#if !defined(__WINDOWS__)
	ASSERT("internal::fill_address(1)\n", status == EAI_SYSTEM);
#endif
	if (status != 0) {
		std::string message("internal::fill_address(2)\n");
		message.append(::gai_strerror(status));
		throw std::runtime_error(message);
	}

	__socket_address = *(struct sockaddr_in*)result->ai_addr;

	freeaddrinfo(result);
}


std::string
internal::sock_addr_string(const struct sockaddr* __address)
{
	if(!__address) return "";

	switch(__address->sa_family) {
		case AF_INET: {
			char buffer[INET_ADDRSTRLEN];
			struct sockaddr_in* socket_address = (struct sockaddr_in*)__address;
			auto result = inet_ntop(
        AF_INET,
        &socket_address->sin_addr,
				buffer,
        INET_ADDRSTRLEN
      );
			if(result == NULL) return "";
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
			if(result == NULL) return "";
			else return std::string(result);
		}

		default: return "";
	};
}
