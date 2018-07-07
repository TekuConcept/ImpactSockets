/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef _GENERIC_H_
#define _GENERIC_H_

#include "Environment.h"
#include "SocketHandle.h"
#include <string>
#include <locale>

#if defined(__WINDOWS__)
    #include <winsock2.h> // sockaddr_in
#else
    #include <netinet/in.h> // sockaddr_in
#endif

namespace Impact {
namespace Internal {

#if defined(__WINDOWS__)
    std::string toNarrowString(const wchar_t* original, char unknown = '?',
        const std::locale& env = std::locale());

	std::string getWinErrorMessage(unsigned long);
#endif /* __WINDOWS__ */


    // int getLastError();
	std::string getErrorMessage();
    // std::string getHostErrorMessage();
    
    void fillAddress(const SocketHandle&, const std::string&,
        unsigned short port, sockaddr_in&)
        /* throw(std::runtime_error) */;

    std::string sockAddr2String(const struct sockaddr*);

}}

#endif