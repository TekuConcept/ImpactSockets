/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef _IMPACT_GENERIC_H_
#define _IMPACT_GENERIC_H_

#include <string>
#include <locale>

#include "sockets/environment.h"
#include "sockets/types.h"

#if defined(__WINDOWS__)
    #include <winsock2.h> // sockaddr_in
#else
    #include <netinet/in.h> // sockaddr_in
#endif

namespace impact {
namespace internal {

#if defined(__WINDOWS__)
  std::string to_narrow_string(const wchar_t* original, char unknown = '?',
    const std::locale& env = std::locale());

	std::string win_error_message(unsigned long code);
#else /* Linux | BSD */
  void no_sigpipe(); /* throw(std::runtime_error) */
#endif /* __WINDOWS__ */

	std::string error_message();
  void fill_address(socket_domain, socket_type, socket_protocol,
		const std::string& host, const unsigned short port, sockaddr_in& result)
    /* throw(std::runtime_error) */;
  std::string sock_addr_string(const struct sockaddr* address);

}}

#endif
