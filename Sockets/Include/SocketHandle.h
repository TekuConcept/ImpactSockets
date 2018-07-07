/**
 * Created by TekuConcept on June 15, 2018
 */

#ifndef _SOCKET_HANDLE_H_
#define _SOCKET_HANDLE_H_

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
		void fillAddress(const SocketHandle&,
			const std::string&, unsigned short port, sockaddr_in&);
		Networking::InterfaceType gniLinuxGetInterfaceType(SocketDomain domain,
			const std::string& interfaceName);
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
		friend void Internal::fillAddress(const SocketHandle&,
			const std::string&, unsigned short port, sockaddr_in&);
		friend Networking::InterfaceType Internal::gniLinuxGetInterfaceType(
			SocketDomain domain, const std::string& interfaceName);
	};
}

#endif