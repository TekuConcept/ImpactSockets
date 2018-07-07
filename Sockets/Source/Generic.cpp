/**
 * Created by TekuConcept on July 7, 2018
 */

#include "Generic.h"
#include <sstream>              // ostringstream
#include <cstring>				// strerror
#include <stdexcept>            // runtime_error

#if defined(__LINUX__)
    #include <netdb.h>          // h_errno
#endif

#if defined(__WINDOWS__)
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
 		message.append(Internal::getErrorMessage());\
 		throw std::runtime_error(message);\
 	}

using namespace Impact;

#if defined(__WINDOWS__)
std::string Internal::toNarrowString(const wchar_t* original, char unknown,
    const std::locale& env) {
	std::ostringstream os;
	while (*original != L'\0')
		os << std::use_facet< std::ctype<wchar_t> >(env).narrow(*original++, unknown);
	return os.str();
}


std::string Internal::getWinErrorMessage(unsigned long code) {
	switch (code) {
	case WSASYSNOTREADY:				return "The underlying network subsystem is not ready for network communication.";
	case WSAVERNOTSUPPORTED:			return "The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation.";
	case WSAEINPROGRESS:				return "A blocking Windows Sockets 1.1 operation is in progress.";
	case WSAEPROCLIM:					return "A limit on the number of tasks supported by the Windows Sockets implementation has been reached.";
	case WSAEFAULT:						return "The lpWSAData parameter is not a valid pointer.";
	case ERROR_ADDRESS_NOT_ASSOCIATED:	return "An address has not yet been associated with the network endpoint.DHCP lease information was available.";
	case ERROR_BUFFER_OVERFLOW:			return "The buffer size indicated by the SizePointer parameter is too small to hold the adapter information or the AdapterAddresses parameter is NULL.The SizePointer parameter returned points to the required size of the buffer to hold the adapter information.";
	case ERROR_INVALID_PARAMETER:		return "One of the parameters is invalid.This error is returned for any of the following conditions : the SizePointer parameter is NULL, the Address parameter is not AF_INET, AF_INET6, or AF_UNSPEC, or the address information for the parameters requested is greater than ULONG_MAX.";
	case ERROR_NOT_ENOUGH_MEMORY:		return "Insufficient memory resources are available to complete the operation.";
	case ERROR_NO_DATA:					return "No addresses were found for the requested parameters.";
	default:
		std::string data(128, '\0');
		auto status = FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, code,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			&data[0], 128, NULL
		);
		if (status == 0) return "[No Error Message Available]";
		else return data;
	}
}
#endif

/*
int Internal::getLastError() {
#if defined(__WINDOWS__)
	return WSAGetLastError();
#else
	return errno;
#endif
}
*/

std::string Internal::getErrorMessage() {
	std::ostringstream os;
#if defined(__WINDOWS__)
	char data[128];
	auto errorCode = WSAGetLastError();
	auto status = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
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

/*
std::string Internal::getHostErrorMessage() {
#if defined(__WINDOWS__)
	return getErrorMessage();
#else
	std::ostringstream os;
	switch(h_errno) {
	case HOST_NOT_FOUND:	os << "[Host Error] Host Not Found"; break;
	case NO_DATA:			os << "[Host Error] No Data"; break;
	case NO_RECOVERY:		os << "[Host Error] No Recovery"; break;
	case TRY_AGAIN:			os << "[Host Error] Try Again"; break;
	default:                os << "[Host Error] Success"; break;
	}
	return os.str();
#endif
}
*/

// TODO: Support IPv6
void Internal::fillAddress(const SocketHandle& handle,
	const std::string& address, unsigned short port,
	sockaddr_in& socketAddress) {

	struct addrinfo hints, *result;
	memset(&hints, 0, sizeof(hints));
	memset(&socketAddress, 0, sizeof(socketAddress));

	hints.ai_family   = AF_INET;//(int)handle.domain;
	hints.ai_socktype = (int)handle.type;
	hints.ai_flags    = AI_PASSIVE;
	hints.ai_protocol = (int)handle.protocol;

	auto sport = std::to_string(port);
	auto status = ::getaddrinfo(&address[0], &sport[0], &hints, &result);
#if !defined(__WINDOWS__)
	ASSERT("SocketInterface::fillAddress(1)\n", status == EAI_SYSTEM);
#endif
	if(status != 0) {
		std::string message("SocketInterface::fillAddress(2)\n");
		message.append(::gai_strerror(status));
		throw std::runtime_error(message);
	}
	socketAddress = *(sockaddr_in*)result->ai_addr;

	freeaddrinfo(result);
}


std::string Internal::sockAddr2String(const struct sockaddr* address) {
	if(!address) return "";
	switch(address->sa_family) {
		case AF_INET: {
			char buffer[INET_ADDRSTRLEN];
			struct sockaddr_in* socketAddress = (struct sockaddr_in*)address;
			auto result = inet_ntop(AF_INET, &socketAddress->sin_addr,
				buffer, INET_ADDRSTRLEN);
			if(result == NULL) return "";
			else return std::string(result);
		}
		case AF_INET6: {
			char buffer[INET6_ADDRSTRLEN];
			struct sockaddr_in6* socketAddress = (struct sockaddr_in6*)address;
			auto result = inet_ntop(AF_INET6, &socketAddress->sin6_addr,
				buffer, INET6_ADDRSTRLEN);
			if(result == NULL) return "";
			else return std::string(result);
			return std::string(buffer);
		}
		default: return "";
	};
}
