/**
 * Created by TekuConcept on June 15, 2018
 */

#ifndef _SOCKET_HANDLE_H_
#define _SOCKET_HANDLE_H_

#include "Environment.h"
#include "SocketTypes.h"
#include "Networking.h"
#include <string>

#if defined(__WINDOWS__)
    #include <winsock2.h> // sockaddr_in
#else
    #include <netinet/in.h> // sockaddr_in
#endif

namespace Impact {
	class SocketHandle;
	namespace Internal {
		#if defined(__LINUX__)
		Networking::InterfaceType getInterfaceType(SocketDomain domain,
			const std::string& interfaceName);
		#endif
	}

	class SocketHandle {
	public:
		SocketHandle();
		virtual ~SocketHandle();

		// SocketHandle& operator=(SocketHandle&& handle);
		// SocketHandle& operator=(SocketHandle const&) = delete;

	protected:
		int descriptor;
		SocketDomain domain;
		SocketType type;
		SocketProtocol protocol;

		/* LIST ALL FRIENDS HERE */
		friend class SocketInterface;
		friend class SocketPollTable;
		#if defined(__LINUX__)
		friend Networking::InterfaceType Internal::getInterfaceType(
			SocketDomain domain, const std::string& interfaceName);
		#endif
	};
}

#endif