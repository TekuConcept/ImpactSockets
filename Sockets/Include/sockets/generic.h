/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef _IMPACT_GENERIC_H_
#define _IMPACT_GENERIC_H_

#include <string>
#include <vector>
#include <locale>

#include "utils/environment.h"
#include "sockets/types.h"

#if defined(__OS_WINDOWS__)
    #include <winsock2.h> // sockaddr_in
#else
    #include <netinet/in.h> // sockaddr_in
#endif

namespace impact {
namespace internal {
    int error_code();
    std::string error_message();
    
    void fill_address(socket_domain, socket_type, socket_protocol,
        const std::string& host, const unsigned short port,
        std::shared_ptr<struct sockaddr>* result)
        /* throw(impact_error) */;
    void fill_address(const std::vector<unsigned char>& address,
        const unsigned short port,
        std::shared_ptr<struct sockaddr>* result)
        /* throw(impact_error) */;

    std::vector<unsigned char> sock_addr_byte_array(
        const struct sockaddr* address)
        /* throw(impact_error) */;

    std::string sock_addr_string(const struct sockaddr* address);

#if defined(__OS_WINDOWS__)
    std::string to_narrow_string(const wchar_t* original, char unknown = '?',
        const std::locale& env = std::locale());

    std::string win_error_message(unsigned long code);

    int wsa_error_string(unsigned long code, std::string& buffer);
#else /* Linux | BSD */
    void no_sigpipe(); /* throw(impact_error) */
#endif /* __OS_WINDOWS__ */
}}

#endif
