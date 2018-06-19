/**
 * Created by TekuConcept on June 15, 2018
 */

#ifndef _SOCKET_HANDLE_H_
#define _SOCKET_HANDLE_H_

#include "SocketTypes.h"

namespace Impact {
	class SocketHandle {
	public:
		SocketHandle(int handle);
		SocketHandle(
			SocketType socketType,
			SocketProtocol protocol,
			SocketDomain domain=SocketDomain::INET)
			/* throw(std::runtime_error) */;
		~SocketHandle();

	protected:
		int descriptor;

		/* LIST ALL FRIEND CLASSES WHO NEED ACCESS HERE */
		friend class SocketInterface;
	};
}

#endif