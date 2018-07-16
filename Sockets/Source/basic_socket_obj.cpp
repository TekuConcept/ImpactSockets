/**
 * Created by TekuConcept on July 11, 2018
 */

#include "sockets/basic_socket.h"
#include "basic_socket_common.inc"

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
		throw std::runtime_error(
      "basic_socket::copy()\n"
      "Copying moved socket"
    );
	m_info_ = __rvalue.m_info_;

	if (m_info_->ref_count > 0)
		(m_info_->ref_count)++; // only increment if valid
}


void
basic_socket::_M_move(basic_socket&& __rvalue)
{
	if (__rvalue.m_info_ == NULL) {
		throw std::runtime_error(
			"basic_socket::basic_socket()\n"
      "Moving already moved object"
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
#if !defined(__WINDOWS__)
	try { internal::no_sigpipe(); } catch (...) { /* do nothing */ }
#endif
	m_info_              = new struct basic_socket_info;
	m_info_->ref_count   = 0;
	m_info_->wsa         = false;
	m_info_->descriptor  = INVALID_SOCKET;
	m_info_->domain      = socket_domain::UNSPECIFIED;
	m_info_->type        = socket_type::RAW;
	m_info_->protocol    = socket_protocol::DEFAULT;
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
	else return INVALID_SOCKET;
}


socket_domain
basic_socket::domain() const noexcept
{
	if (m_info_) return m_info_->domain;
	else return socket_domain::UNSPECIFIED;
}


socket_type
basic_socket::type() const noexcept
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
