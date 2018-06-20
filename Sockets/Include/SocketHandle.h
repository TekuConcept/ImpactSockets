/**
 * Created by TekuConcept on June 15, 2018
 */

#ifndef _SOCKET_HANDLE_H_
#define _SOCKET_HANDLE_H_

#include "SocketTypes.h"

namespace Impact {
	class SocketHandle {
	public:
		SocketHandle(SocketHandle const&) = delete;
		SocketHandle(int handle);
		SocketHandle(SocketHandle&& handle);
		SocketHandle(
			SocketType socketType,
			SocketProtocol protocol,
			SocketDomain domain=SocketDomain::INET)
			/* throw(std::runtime_error) */;
		virtual ~SocketHandle();

		SocketHandle& operator=(SocketHandle&& handle);
		SocketHandle& operator=(SocketHandle const&) = delete;

	protected:
		int descriptor;

		/* LIST ALL FRIEND CLASSES WHO NEED ACCESS HERE */
		friend class SocketInterface;
		friend class SocketPollTable;

	private:
		void move(SocketHandle& rhs);
	};
}

#endif