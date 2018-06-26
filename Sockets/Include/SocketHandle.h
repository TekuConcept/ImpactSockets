/**
 * Created by TekuConcept on June 15, 2018
 */

#ifndef _SOCKET_HANDLE_H_
#define _SOCKET_HANDLE_H_

#include "SocketTypes.h"

namespace Impact {
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

		/* LIST ALL FRIEND CLASSES WHO NEED ACCESS HERE */
		friend class SocketInterface;
		friend class SocketPollTable;
	};
}

#endif