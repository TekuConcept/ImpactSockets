/**
 * Created by TekuConcept on July 7, 2018
 */

#include "basic_socket.h"
#include "io_error.h"
#include "Generic.h"

#include <sys/types.h>			 // For data types
#include <cstring>				   // For strerror and memset
#include <stdlib.h>				   // For atoi
#include <errno.h>				   // For errno
#include <sstream>
#include <stdexcept>

#if defined(__WINDOWS__)
	#pragma pop_macro("IN")    // pushed in SocketTypes.h
	#pragma pop_macro("OUT")   // pushed in SocketTypes.h
	#pragma pop_macro("ERROR") // pushed in SocketTypes.h
	#include <winsock2.h>
	#include <ws2tcpip.h>
 	#include <mstcpip.h>       // struct tcp_keepalive
	#include <iphlpapi.h>
#else
	#include <sys/socket.h>		 // For socket(), connect(), send(), and recv()
	#include <netdb.h>			   // For gethostbyname()
	#include <arpa/inet.h>		 // For inet_addr(), ntohs()
	#include <unistd.h>			   // For close()
 	#include <netinet/tcp.h>	 // For IPPROTO_TCP, TCP_KEEPCNT, TCP_KEEPINTVL,
 								             // TCP_KEEPIDLE
  #include <sys/ioctl.h>     // For ioctl()
  #include <net/if.h>        // For ifconf
  #include <sys/poll.h>      // For struct pollfd, poll()
  #include <netinet/in.h>    // For sockaddr_in
  #include <ifaddrs.h>       // getifaddrs(), freeifaddrs()
#if defined(__APPLE__)
	#include <net/if_types.h>  // For IFT_XXX types
	#include <net/if_dl.h>     // For sockaddr_dl
#endif
#endif

#if defined(__LINUX__)
	#include <net/if_arp.h>
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
 		message.append(Internal::getErrorMessage());\
 		throw io_error(message);\
 	}

#define WIN_ASSERT(title,cond,error,fin)\
	if(cond) {\
		fin\
		std::string message( title );\
		message.append(Internal::getWinErrorMessage( error ));\
		throw io_error(message);\
	}


using namespace Impact;


void basic_socket::local_port(unsigned short port) {
  sockaddr_in socketAddress;
	::memset(&socketAddress, 0, sizeof(socketAddress));
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	socketAddress.sin_port = htons(port);
	auto status = ::bind(_descriptor_, (sockaddr*)&socketAddress,
		sizeof(sockaddr_in));

	ASSERT("basic_socket::local_port()\n", status == SOCKET_ERROR);
}


void basic_socket::local_address_port(const std::string& address,
  unsigned short port) {
  sockaddr_in socketAddress;

	CATCH_ASSERT(
		"basic_socket::local_address_port(1)\n",
		Internal::fillAddress(_domain_, _type_, _protocol_, address, port,
      socketAddress);
	);

	auto status = ::bind(_descriptor_, (sockaddr*)&socketAddress,
		sizeof(sockaddr_in));

	ASSERT("basic_socket::local_address_port(2)\n", status == SOCKET_ERROR);
}


unsigned short basic_socket::resolve_service(const std::string& service,
  const std::string& protocol) {
  struct servent* serviceInfo = ::getservbyname(service.c_str(),
		protocol.c_str());

	if (serviceInfo == NULL)
		return static_cast<unsigned short>(atoi(service.c_str()));
		/* Service is port number */
	else return ntohs(serviceInfo->s_port);
		/* Found port (network byte order) by name */
}


bool basic_socket::close() {
  auto status = CLOSE_SOCKET(_descriptor_);
	_descriptor_ = INVALID_SOCKET;
#if defined(__WINDOWS__)
	WSACleanup();
#endif
  return status == SOCKET_ERROR;
}


basic_socket::basic_socket() {
  _ref_ = new long; *_ref_ = 1;
}


basic_socket::basic_socket(SocketDomain domain, SocketType type,
  SocketProtocol proto) : _domain_(domain), _type_(type), _protocol_(proto) {
  _ref_ = new long; *_ref_ = 1;
#if defined(__WINDOWS__)
	static WSADATA wsaData;
	auto status = WSAStartup(MAKEWORD(2, 2), &wsaData);
	WIN_ASSERT("SocketInterface::create(1)\n", status != 0, status, (void)0;);
#endif
	_descriptor_ = ::socket((int)domain, (int)type, (int)proto);
	ASSERT(
		"SocketInterface::create(2)\n",
		_descriptor_ == INVALID_SOCKET
	);
}


basic_socket::basic_socket(const basic_socket& r) {
  CATCH_ASSERT("basic_socket::basic_socket()\n", *this = r;)
}


basic_socket::basic_socket(basic_socket&& r) {
  CATCH_ASSERT("basic_socket::basic_socket()\n", *this = std::move(r);)
}


basic_socket::~basic_socket() {
  if(_ref_ != NULL) {
    if(*_ref_ == 1) {
      close();
      delete _ref_;
      _ref_ = NULL;
    }
    else (*_ref_)--;
  }
  // else moved
}


basic_socket& basic_socket::operator=(const basic_socket& r) {
  _ref_ = r._ref_;
  if(_ref_ == NULL) {
    throw std::runtime_error(
      "basic_socket::operator=()\n"
      "Copying moved object"
    );
  }
  (*_ref_)++;
  _domain_     = r._domain_;
  _type_       = r._type_;
  _protocol_   = r._protocol_;
  _descriptor_ = r._descriptor_;
  return *this;
}


basic_socket& basic_socket::operator=(basic_socket&& r) {
  _ref_ = r._ref_;
  if(_ref_ == NULL) {
    throw std::runtime_error(
      "basic_socket::operator=()\n"
      "Moving already moved object"
    );
  }
  r._ref_      = NULL;
  _domain_       = r._domain_;
  _type_         = r._type_;
  _protocol_     = r._protocol_;
  _descriptor_   = r._descriptor_;
  r._descriptor_ = INVALID_SOCKET;
  return *this;
}


long basic_socket::use_count() const noexcept {
  if (_ref_ == NULL) return 0;
  else return *_ref_;
  return *_ref_;
}


int basic_socket::get() const noexcept {
  return _descriptor_;
}


SocketDomain basic_socket::domain() const noexcept {
  return _domain_;
}


SocketType basic_socket::type() const noexcept {
  return _type_;
}


SocketProtocol basic_socket::protocol() const noexcept {
  return _protocol_;
}


basic_socket::operator bool() const noexcept {
  return _descriptor_ == INVALID_SOCKET;
}


void basic_socket::connect(unsigned short port, const std::string& address) {
	sockaddr_in destinationAddress;

	CATCH_ASSERT(
		"basic_socket::connect(1)\n",
		Internal::fillAddress(_domain_, _type_, _protocol_, address, port,
      destinationAddress);
	);

	auto status = ::connect(_descriptor_, (sockaddr*)&destinationAddress,
		sizeof(destinationAddress));

	ASSERT("basic_socket::connect(2)\n", status == SOCKET_ERROR);
}


void basic_socket::listen(int backlog) {
  auto status = ::listen(_descriptor_, backlog);
  ASSERT("basic_socket::listen()\n", status == SOCKET_ERROR);
}


basic_socket basic_socket::accept() {
  basic_socket peer;
  peer._domain_     = _domain_;
  peer._type_       = _type_;
  peer._protocol_   = _protocol_;
  peer._descriptor_ = ::accept(_descriptor_, NULL, NULL);
	ASSERT("basic_socket::accept()\n", peer._descriptor_ == INVALID_SOCKET);
  return peer;
}


void basic_socket::shutdown(SocketChannel channel) {
  auto status = ::shutdown(_descriptor_, (int)channel);
  ASSERT("basic_socket::shutdown()\n", status == SOCKET_ERROR);
}


void basic_socket::group(std::string multicastName, GroupApplication method) {
	struct ip_mreq multicastRequest;
	multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastName.c_str());
	multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
	auto status = ::setsockopt(_descriptor_, IPPROTO_IP, (int)method,
		(CCHAR_PTR)&multicastRequest, sizeof(multicastRequest));
	ASSERT("basic_socket::group()\n", status == SOCKET_ERROR);
}


void basic_socket::keepalive(KeepAliveOptions options) {
  // http://helpdoco.com/C++-C/how-to-use-tcp-keepalive.htm
	std::ostringstream os("basic_socket::keepalive()\n");
	auto errors = 0;
	auto status = 0;
#if defined(__WINDOWS__)
	DWORD bytesReturned = 0;
	struct tcp_keepalive config;
	config.onoff             = options.enabled;
	config.keepalivetime     = options.idleTime;
	config.keepaliveinterval = options.interval;
	status = WSAIoctl(_descriptor_, SIO_KEEPALIVE_VALS, &config,
		sizeof(config), NULL, 0, &bytesReturned, NULL, NULL);
	if (status == SOCKET_ERROR) {
		os << Internal::getErrorMessage();
		throw io_error(os.str());
	}
#else /* OSX|LINUX */
	status = setsockopt(_descriptor_, SOL_SOCKET, SO_KEEPALIVE,
		(const char*)&options.enabled, sizeof(int));
	if(status == SOCKET_ERROR) {
		os << "[keepalive] ";
		os << Internal::getErrorMessage();
		os << std::endl;
		errors |= 1;
	}
#ifndef __APPLE__
	status = setsockopt(_descriptor_, IPPROTO_TCP, TCP_KEEPIDLE,
		(const char*)&options.idleTime, sizeof(int));
	if(status == SOCKET_ERROR) {
		os << "[idle] ";
		os << Internal::getErrorMessage();
		os << std::endl;
		errors |= 8;
	}
#endif /* __APPLE__ */
	status = setsockopt(_descriptor_, IPPROTO_TCP, TCP_KEEPINTVL,
		(const char*)&options.interval, sizeof(int));
	if(status == SOCKET_ERROR) {
		os << "[interval] ";
		os << Internal::getErrorMessage();
		os << std::endl;
		errors |= 2;
	}
#endif /* UNIX|LINUX */
	status = setsockopt(_descriptor_, IPPROTO_TCP, TCP_KEEPCNT,
		(const char*)&options.retries, sizeof(int));
	if(status == SOCKET_ERROR) {
		os << "[count] ";
		os << Internal::getErrorMessage();
		os << std::endl;
		errors |= 4;
	}
	if(errors) throw io_error(os.str());
}


void basic_socket::send(const void* buffer, int length, MessageFlags flags) {
  auto status = ::send(_descriptor_, (CCHAR_PTR)buffer, length, (int)flags);
  ASSERT("basic_socket::send()\n", status == SOCKET_ERROR);
}


int basic_socket::sendto(const void* buffer, int length, unsigned short port,
  const std::string& address, MessageFlags flags) {
  sockaddr_in destinationAddress;

	CATCH_ASSERT(
		"basic_socket::sendto(1)\n",
		Internal::fillAddress(_domain_, _type_, _protocol_, address, port,
      destinationAddress);
	);

	auto status = ::sendto(_descriptor_, (CCHAR_PTR)buffer, length,
		(int)flags, (sockaddr*)&destinationAddress, sizeof(destinationAddress));

	ASSERT("basic_socket::sendto(2)\n", status == SOCKET_ERROR);
	return status;
}


int basic_socket::recv(void* buffer, int length, MessageFlags flags) {
  int status = ::recv(_descriptor_, (CHAR_PTR)buffer, length, (int)flags);
	ASSERT("basic_socket::recv()\n", status == SOCKET_ERROR);
	return status; /* number of bytes received or EOF */
}


int basic_socket::recvfrom(void* buffer, int length, unsigned short& port,
  std::string& address, MessageFlags flags) {
  sockaddr_in clientAddress;
	socklen_t addressLength = sizeof(clientAddress);
	auto status = ::recvfrom(_descriptor_, (CHAR_PTR)buffer, length,
		(int)flags, (sockaddr*)&clientAddress, (socklen_t*)&addressLength);

	ASSERT("basic_socket::recvfrom()\n", status == SOCKET_ERROR);

	address = inet_ntoa(clientAddress.sin_addr);
	port    = ntohs(clientAddress.sin_port);

	return status;
}


std::string basic_socket::local_address() {
  sockaddr_in address;
	auto addressLength = sizeof(address);
	auto status = ::getsockname(_descriptor_,
		(sockaddr*)&address, (socklen_t*)&addressLength);
	ASSERT("basic_socket::local_address()\n", status == SOCKET_ERROR);
	return inet_ntoa(address.sin_addr);
}


unsigned short basic_socket::local_port() {
  sockaddr_in address;
	auto addressLength = sizeof(address);
	auto status = ::getsockname(_descriptor_,
		(sockaddr*)&address, (socklen_t*)&addressLength);
	ASSERT("basic_socket::local_port()\n", status == SOCKET_ERROR);
	return ntohs(address.sin_port);
}


std::string basic_socket::peer_address() {
  sockaddr_in address;
  unsigned int addressLength = sizeof(address);
  auto status = ::getpeername(_descriptor_, (sockaddr*)&address,
    (socklen_t*)&addressLength);
  ASSERT("basic_socket::peer_address()\n", status == SOCKET_ERROR);
  return inet_ntoa(address.sin_addr);
}


unsigned short basic_socket::peer_port() {
  sockaddr_in address;
  unsigned int addressLength = sizeof(address);
  auto status = ::getpeername(_descriptor_, (sockaddr*)&address,
    (socklen_t*)&addressLength);
  ASSERT("basic_socket::peer_port()\n", status == SOCKET_ERROR);
  return ntohs(address.sin_port);
}


void basic_socket::broadcast(bool enabled) {
  auto permission = enabled?1:0;
	auto status = ::setsockopt(_descriptor_, SOL_SOCKET, SO_BROADCAST,
		(CCHAR_PTR)&permission, sizeof(permission));
	ASSERT("basic_socket::broadcast()\n", status == SOCKET_ERROR);
}


void basic_socket::multicast_ttl(unsigned char ttl) {
  auto status = ::setsockopt(_descriptor_, IPPROTO_IP, IP_MULTICAST_TTL,
		(CCHAR_PTR)&ttl, sizeof(ttl));
	ASSERT("basic_socket::multicast_ttl()\n", status == SOCKET_ERROR);
}
