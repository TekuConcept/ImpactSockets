/**
 * Created by TekuConcept on July 7, 2018
 */

#include "sockets/basic_socket.h"

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


using namespace impact;


unsigned short
basic_socket::_M_resolve_service(
	const std::string& __service,
	const std::string& __protocol)
{
	struct servent* service_info = ::getservbyname(__service.c_str(),
		__protocol.c_str());
	if (service_info == NULL)
		return static_cast<unsigned short>(atoi(__service.c_str()));
	/* Type of service is the port number ie 80/http*/
	else return ntohs(service_info->s_port);
	/* Found port (network byte order) by name */
}


void
basic_socket::_M_copy(const basic_socket& __rvalue)
{
	if (__rvalue.m_info_ == NULL)
		throw std::runtime_error("basic_socket::copy()\nCopying moved socket");
	m_info_ = __rvalue.m_info_;

	if (m_info_->ref_count > 0)
		(m_info_->ref_count)++; // only increment if valid
}


void
basic_socket::_M_move(basic_socket&& __rvalue)
{
	if (__rvalue.m_info_ == NULL) {
		throw std::runtime_error(
			"basic_socket::basic_socket()\nMoving already moved object"
		);
	}
	m_info_          = __rvalue.m_info_;
	__rvalue.m_info_ = NULL;
}


void
basic_socket::_M_dtor()
{
	if (m_info_ != NULL) {
		if (m_info_->ref_count == 1) {
			try { close(); }
			catch (...) { throw; }
			delete m_info_;
			m_info_ = NULL;
		}
		else if (m_info_->ref_count > 1)
			m_info_->ref_count--;
	}
	// else moved
}


basic_socket::basic_socket()
{
	m_info_             = new struct basic_socket_info;
	m_info_->ref_count  = 0;
	m_info_->wsa        = false;
	m_info_->descriptor = INVALID_SOCKET;
	m_info_->domain     = socket_domain::UNSPECIFIED;
	m_info_->type       = socket_type::RAW;
	m_info_->protocol   = socket_protocol::DEFAULT;
}


basic_socket::basic_socket(const basic_socket& __rvalue)
{
	CATCH_ASSERT("basic_socket::basic_socket()\n", _M_copy(__rvalue);)
}


basic_socket::basic_socket(basic_socket&& __rvalue)
{
	CATCH_ASSERT("basic_socket::basic_socket()\n", _M_move(std::move(__rvalue));)
}


basic_socket
impact::make_socket(
	socket_domain   __domain,
	socket_type     __type,
	socket_protocol __proto)
{
	basic_socket result;
	#if defined(__WINDOWS__)
		static WSADATA wsa_data;
		auto status = WSAStartup(MAKEWORD(2, 2), &wsa_data);
		WIN_ASSERT("::make_socket(1)\n", status != 0, status, (void)0;);
		result.m_info_->wsa        = true;
	#endif
		result.m_info_->descriptor = ::socket((int)__domain, (int)__type, (int)__proto);
		ASSERT(
			"basic_socket::make_socket(2)\n",
			result.m_info_->descriptor == INVALID_SOCKET
		);
		result.m_info_->ref_count  = 1;
		result.m_info_->domain     = __domain;
		result.m_info_->type       = __type;
		result.m_info_->protocol   = __proto;
	return result;
}


basic_socket
impact::make_tcp_socket()
{
	CATCH_ASSERT(
		"::make_tcp_socket()\n",
		return make_socket(
			socket_domain::INET,
			socket_type::STREAM,
			socket_protocol::TCP
		);
	)
}


basic_socket
impact::make_udp_socket()
{
	CATCH_ASSERT(
		"::make_udp_socket()\n",
		return make_socket(
			socket_domain::INET,
			socket_type::DATAGRAM,
			socket_protocol::UDP
		);
	)
}


basic_socket::~basic_socket()
{
  try { _M_dtor(); } catch (...) { /* do nothing */ }
}


void
basic_socket::close()
{
	ASSERT_MOVED("basic_socket::close()\n");
	auto status = CLOSE_SOCKET(m_info_->descriptor);
	ASSERT("basic_socket::close()\n", status == SOCKET_ERROR);
	m_info_->ref_count  = 0;
	m_info_->descriptor = INVALID_SOCKET;
#if defined(__WINDOWS__)
	if (m_info_->wsa)
		WSACleanup();
	m_info_->wsa        = false;
#endif
}


basic_socket&
basic_socket::operator=(const basic_socket& __rvalue)
{
	if (m_info_ && m_info_->ref_count > 0)
		_M_dtor();
	CATCH_ASSERT("basic_socket::operator=()\n", _M_copy(__rvalue);)
	return *this;
}


basic_socket&
basic_socket::operator=(basic_socket&& __rvalue)
{
	if (m_info_ && m_info_->ref_count > 0)
		_M_dtor();
	CATCH_ASSERT("basic_socket::operator=()\n", _M_move(std::move(__rvalue));)
	return *this;
}


long
basic_socket::use_count() const noexcept
{
  if (m_info_)
	  return m_info_->ref_count;
  else return 0;
}


int
basic_socket::get() const noexcept
{
	if (m_info_)
		return m_info_->descriptor;
	else return 0;
}


socket_domain
basic_socket::domain() const noexcept
{
	if (m_info_) return m_info_->domain;
	else return socket_domain::UNSPECIFIED;
}


socket_type basic_socket::type() const noexcept
{
	if (m_info_) return m_info_->type;
	else return socket_type::RAW;
}


socket_protocol
basic_socket::protocol() const noexcept
{
	if (m_info_) return m_info_->protocol;
	else return socket_protocol::DEFAULT;
}


basic_socket::operator bool() const noexcept
{
	if (m_info_)
		return m_info_->descriptor != INVALID_SOCKET;
	else return false;
}


void
basic_socket::bind(unsigned short __port)
{
	ASSERT_MOVED("basic_socket::bind()\n");
	struct sockaddr_in socket_address;

	::memset(&socket_address, 0, sizeof(socket_address));
	socket_address.sin_family      = AF_INET;
	socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
	socket_address.sin_port        = htons(__port);

	auto status = ::bind(
		m_info_->descriptor,
		(struct sockaddr*)&socket_address,
		sizeof(socket_address)
	);

	ASSERT("basic_socket::bind()\n", status == SOCKET_ERROR);
}


void
basic_socket::bind(
	const std::string& __address,
	unsigned short     __port)
{
	ASSERT_MOVED("basic_socket::bind()\n");
	struct sockaddr_in socket_address;

	CATCH_ASSERT(
		"basic_socket::bind(1)\n",
		internal::fill_address(
			m_info_->domain,
			m_info_->type,
			m_info_->protocol,
			__address,
			__port,
			socket_address
		);
	);

	auto status = ::bind(
		m_info_->descriptor,
		(struct sockaddr*)&socket_address,
		sizeof(socket_address)
	);

	ASSERT("basic_socket::bind(2)\n", status == SOCKET_ERROR);
}


void
basic_socket::connect(
	unsigned short     __port,
	const std::string& __address)
{
	ASSERT_MOVED("basic_socket::connect()\n");
	struct sockaddr_in destination_address;

	CATCH_ASSERT(
		"basic_socket::connect(1)\n",
		internal::fill_address(
			m_info_->domain,
			m_info_->type,
			m_info_->protocol,
			__address,
			__port,
			destination_address
		);
	);

	auto status = ::connect(
		m_info_->descriptor,
		(struct sockaddr*)&destination_address,
		sizeof(destination_address)
	);

	ASSERT("basic_socket::connect(2)\n", status == SOCKET_ERROR);
}


void
basic_socket::listen(int __backlog)
{
	ASSERT_MOVED("basic_socket::listen()\n");
	auto status = ::listen(m_info_->descriptor, __backlog);
	ASSERT("basic_socket::listen()\n", status == SOCKET_ERROR);
}


basic_socket
basic_socket::accept()
{
	ASSERT_MOVED("basic_socket::accept()\n");
	basic_socket peer;
	peer.m_info_->descriptor = ::accept(m_info_->descriptor, NULL, NULL);
	ASSERT("basic_socket::accept()\n",
		peer.m_info_->descriptor == INVALID_SOCKET);
	peer.m_info_->ref_count  = 1;
	peer.m_info_->wsa        = false;
	peer.m_info_->domain     = m_info_->domain;
	peer.m_info_->type       = m_info_->type;
	peer.m_info_->protocol   = m_info_->protocol;
	return peer;
}


void
basic_socket::shutdown(socket_channel __channel)
{
	ASSERT_MOVED("basic_socket::shutdown()\n");
	auto status = ::shutdown(m_info_->descriptor, (int)__channel);
	ASSERT("basic_socket::shutdown()\n", status == SOCKET_ERROR);
}


void
basic_socket::group(
	std::string            __name,
	group_application      __method)
{
	ASSERT_MOVED("basic_socket::group()\n");
	struct ip_mreq multicast_request;
	multicast_request.imr_multiaddr.s_addr = inet_addr(__name.c_str());
	multicast_request.imr_interface.s_addr = htonl(INADDR_ANY);
	auto status = ::setsockopt(
		m_info_->descriptor,
		IPPROTO_IP,
		(int)__method,
		(CCHAR_PTR)&multicast_request,
		sizeof(multicast_request)
	);
	ASSERT("basic_socket::group()\n", status == SOCKET_ERROR);
}


void
basic_socket::keepalive(struct keep_alive_options __options)
{
	ASSERT_MOVED("basic_socket::keepalive()\n");
	// http://helpdoco.com/C++-C/how-to-use-tcp-keepalive.htm
	std::ostringstream os("basic_socket::keepalive()\n");
	auto errors = 0;
	auto status = 0;
#if defined(__WINDOWS__)
	DWORD bytes_returned     = 0;
	struct tcp_keepalive config;
	config.onoff             = __options.enabled;
	config.keepalivetime     = __options.idletime;
	config.keepaliveinterval = __options.interval;

	status = WSAIoctl(
		m_info_->descriptor,
		SIO_KEEPALIVE_VALS,
		&config,
		sizeof(config),
		NULL,
		0,
		&bytes_returned,
		NULL,
		NULL
	);

	if (status == SOCKET_ERROR) {
		os << internal::error_message();
		throw io_error(os.str());
	}
#else /* OSX|LINUX */
	status = setsockopt(
		m_info_->descriptor,
		SOL_SOCKET,
		SO_KEEPALIVE,
		(const char*)&__options.enabled,
		sizeof(int)
	);

	if (status == SOCKET_ERROR) {
		os << "[keepalive] ";
		os << internal::error_message();
		os << std::endl;
		errors |= 1;
	}
#ifndef __APPLE__
	status = setsockopt(
		m_info_->descriptor,
		IPPROTO_TCP,
		TCP_KEEPIDLE,
		(const char*)&__options.idletime,
		sizeof(int)
	);

	if (status == SOCKET_ERROR) {
		os << "[idle] ";
		os << internal::error_message();
		os << std::endl;
		errors |= 8;
	}
#endif /* __APPLE__ */
	status = setsockopt(
		m_info_->descriptor,
		IPPROTO_TCP,
		TCP_KEEPINTVL,
		(const char*)&__options.interval,
		sizeof(int)
	);

	if (status == SOCKET_ERROR) {
		os << "[interval] ";
		os << internal::error_message();
		os << std::endl;
		errors |= 2;
	}
#endif /* UNIX|LINUX */
	status = setsockopt(
		m_info_->descriptor,
		IPPROTO_TCP,
		TCP_KEEPCNT,
		(const char*)&__options.retries,
		sizeof(int)
	);

	if (status == SOCKET_ERROR) {
		os << "[count] ";
		os << internal::error_message();
		os << std::endl;
		errors |= 4;
	}

	if (errors) throw io_error(os.str());
}


void
basic_socket::send(
	const void*        __buffer,
	int                __length,
	message_flags      __flags)
{
	ASSERT_MOVED("basic_socket::send()\n");
	auto status = ::send(
		m_info_->descriptor,
		(CCHAR_PTR)__buffer,
		__length,
		(int)__flags
	);
	ASSERT("basic_socket::send()\n", status == SOCKET_ERROR);
}


int
basic_socket::sendto(
	const void*        __buffer,
	int                __length,
	unsigned short     __port,
	const std::string& __address,
	message_flags      __flags)
{
	ASSERT_MOVED("basic_socket::sendto()\n");
	struct sockaddr_in destination_address;

	CATCH_ASSERT(
		"basic_socket::sendto(1)\n",
		internal::fill_address(
			m_info_->domain,
			m_info_->type,
			m_info_->protocol,
			__address,
			__port,
			destination_address
		);
	);

	auto status = ::sendto(
		m_info_->descriptor,
		(CCHAR_PTR)__buffer,
		__length,
		(int)__flags,
		(struct sockaddr*)&destination_address,
		sizeof(destination_address)
	);

	ASSERT("basic_socket::sendto(2)\n", status == SOCKET_ERROR);
	return status;
}


int
basic_socket::recv(
	void*              __buffer,
	int                __length,
	message_flags      __flags)
{
	ASSERT_MOVED("basic_socket::recv()\n");
	int status = ::recv(
		m_info_->descriptor,
		(CHAR_PTR)__buffer,
		__length,
		(int)__flags
	);
	ASSERT("basic_socket::recv()\n", status == SOCKET_ERROR);
	return status; /* number of bytes received or EOF */
}


int
basic_socket::recvfrom(
	void*              __buffer,
	int                __length,
	unsigned short&    __port,
	std::string&       __address,
	message_flags      __flags)
{
	ASSERT_MOVED("basic_socket::recvfrom()\n");
	struct sockaddr_in client_address;
	socklen_t address_length = sizeof(client_address);

	auto status = ::recvfrom(
		m_info_->descriptor,
		(CHAR_PTR)__buffer,
		__length,
		(int)__flags,
		(struct sockaddr*)&client_address,
		(socklen_t*)&address_length
	);

	ASSERT("basic_socket::recvfrom()\n", status == SOCKET_ERROR);

	__address = inet_ntoa(client_address.sin_addr);
	__port    = ntohs(client_address.sin_port);

	return status;
}


std::string
basic_socket::local_address()
{
	ASSERT_MOVED("basic_socket::local_address()\n");
	struct sockaddr_in address;
	auto address_length = sizeof(address);

	auto status = ::getsockname(
		m_info_->descriptor,
		(struct sockaddr*)&address,
		(socklen_t*)&address_length
	);

	ASSERT("basic_socket::local_address()\n", status == SOCKET_ERROR);
	return inet_ntoa(address.sin_addr);
}


unsigned short
basic_socket::local_port()
{
	ASSERT_MOVED("basic_socket::local_port()\n");
	sockaddr_in address;
	auto address_length = sizeof(address);

	auto status = ::getsockname(
		m_info_->descriptor,
		(struct sockaddr*)&address,
		(socklen_t*)&address_length
	);

	ASSERT("basic_socket::local_port()\n", status == SOCKET_ERROR);
	return ntohs(address.sin_port);
}


std::string
basic_socket::peer_address()
{
	ASSERT_MOVED("basic_socket::peer_address()\n");
	struct sockaddr_in address;
	unsigned int address_length = sizeof(address);

	auto status = ::getpeername(
		m_info_->descriptor,
		(struct sockaddr*)&address,
		(socklen_t*)&address_length
	);

	ASSERT("basic_socket::peer_address()\n", status == SOCKET_ERROR);
	return inet_ntoa(address.sin_addr);
}


unsigned short
basic_socket::peer_port()
{
	ASSERT_MOVED("basic_socket::peer_port()\n");
	struct sockaddr_in address;
	unsigned int address_length = sizeof(address);

	auto status = ::getpeername(
		m_info_->descriptor,
		(struct sockaddr*)&address,
		(socklen_t*)&address_length
	);

	ASSERT("basic_socket::peer_port()\n", status == SOCKET_ERROR);
	return ntohs(address.sin_port);
}


void
basic_socket::broadcast(bool __enabled)
{
	ASSERT_MOVED("basic_socket::broadcast()\n");
	auto permission = __enabled ? 1 : 0;

	auto status = ::setsockopt(
		m_info_->descriptor,
		SOL_SOCKET,
		SO_BROADCAST,
		(CCHAR_PTR)&permission,
		sizeof(permission)
	);

	ASSERT("basic_socket::broadcast()\n", status == SOCKET_ERROR);
}


void
basic_socket::multicast_ttl(unsigned char __ttl)
{
	ASSERT_MOVED("basic_socket::multicast_ttl()\n");
	auto status = ::setsockopt(
		m_info_->descriptor,
		IPPROTO_IP,
		IP_MULTICAST_TTL,
		(CCHAR_PTR)&__ttl,
		sizeof(__ttl)
	);

	ASSERT("basic_socket::multicast_ttl()\n", status == SOCKET_ERROR);
}


void
basic_socket::reuse_address(bool __enabled)
{
	ASSERT_MOVED("basic_socket::reuse_address()\n");
	int reuse = __enabled ? 1 : 0;
	auto option = SO_REUSEADDR;
#ifdef WIN_SECURE_REUSE
	option = SO_EXCLUSIVEADDRUSE;
#endif
	auto status = ::setsockopt(
		m_info_->descriptor,
		SOL_SOCKET,
		option,
		(const char*)&reuse,
		sizeof(reuse)
	);
	ASSERT("basic_socket::reuse_address()\n", status == SOCKET_ERROR);
#ifdef SO_REUSEPORT
	status = ::setsockopt(
		m_info_->descriptor,
		SOL_SOCKET,
		SO_REUSEPORT,
		(const char*)&reuse,
		sizeof(reuse)
	);
	ASSERT("basic_socket::reuse_address()\n", status == SOCKET_ERROR);
#endif
}
