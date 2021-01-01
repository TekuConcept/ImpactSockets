/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef IMPACT_GENERIC_H
#define IMPACT_GENERIC_H

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
    
    size_t fill_address(address_family, socket_type, internet_protocol,
        const std::string& host, const unsigned short port,
        std::shared_ptr<struct sockaddr>* result)
        /* throw(impact_error) */;

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
