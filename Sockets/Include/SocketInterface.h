/**
 * Created by TekuConcept on June 15, 2018
 */

#ifndef _SOCKET_INTERFACE_H_
#define _SOCKET_INTERFACE_H_

#include <iostream>
#include <string>            // For string
#include <cstring>           // For strerror, atoi, and memset
#include <exception>         // For exception class

#include "SocketHandle.h"

#if defined(_MSC_VER)
	#include <winsock2.h>
	// #define SOC_EXCEPTION ...
#else
	#include <sys/poll.h>    // For struct pollfd, poll()
	// #define SOC_EXCEPTION SocketException
#endif

namespace Impact {
	class SocketInterface {
		SocketInterface(); // static-only class

		static std::string getErrorMessage();

	public:
		static std::string getLocalAddress(SocketHandle handle)
			/* throw(std::runtime_error) */;
	};
}

#endif
