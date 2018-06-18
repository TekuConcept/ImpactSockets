/**
 * Created by TekuConcept on June 15, 2018
 */

#ifndef _SOCKET_HANDLE_H_
#define _SOCKET_HANDLE_H_

namespace Impact {
	class SocketHandle {
	public:
		SocketHandle(int handle);
		SocketHandle(int socketType, int protocol)
			/* throw(std::runtime_error) */;
		~SocketHandle();

	protected:
		int descriptor;

		/* LIST ALL FRIEND CLASSES WHO NEED ACCESS HERE */
		friend class SocketInterface;
	};
}

#endif