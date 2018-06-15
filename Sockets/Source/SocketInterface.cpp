/**
 * Created by TekuConcept on June 15, 2018
 */

#include "SocketInterface.h"

#include <sys/types.h>       // For data types#include <cstring>           // For strerror and memset
#include <stdlib.h>          // For atoi
#include <errno.h>           // For errno
#include <stdexcept>         // For std::runtime_error
#include <sstream>

#if defined(_MSC_VER)
	#include <ws2tcpip.h>
#else
	#include <sys/socket.h>  // For socket(), connect(), send(), and recv()
	#include <netdb.h>       // For gethostbyname()
	#include <arpa/inet.h>   // For inet_addr()
	#include <unistd.h>      // For close()
	#include <netinet/in.h>  // For sockaddr_in
#endif

#if defined(_MSC_VER)
	#define SOC_POLL WSAPoll
	#define CLOSE_SOCKET(x) closesocket(x)
	#define SOC_SD_HOW SD_BOTH
	#define CCHAR_PTR const char *
	#define CHAR_PTR char *

	#pragma warning(disable:4996)
	#pragma comment (lib, "Ws2_32.lib")
	#pragma comment (lib, "Mswsock.lib")
	#pragma comment (lib, "AdvApi32.lib")
#else
	#define SOC_POLL ::poll
	#define CLOSE_SOCKET(x) ::close(x)
	#define SOC_SD_HOW SHUT_RDWR
	#define CCHAR_PTR raw_type *
	#define CHAR_PTR raw_type *

 	#define SOCKET_ERROR -1
#endif

typedef void raw_type;       // Type used for raw data on this platform

using namespace Impact;

SocketInterface::SocketInterface() {}


std::string SocketInterface::getErrorMessage() {
	std::ostringstream os;
#if defined(_MSC_VER)
	char data[128];
	auto errorCode = WSAGetLastError();
	auto status = FromatMessage(
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


std::string SocketInterface::getLocalAddress(SocketHandle handle) {
	sockaddr_in address;
	auto addressLength = sizeof(address);
	auto status = ::getsockname(handle.descriptor,
		(sockaddr*)&address, (socklen_t*)&addressLength);

	if(status == SOCKET_ERROR) {
		std::string message("SocketInterface::getLocalAddress() ");
		message.append(getErrorMessage());
		throw std::runtime_error(message);
	}
	return inet_ntoa(address.sin_addr);
}