/**
 * Created by TekuConcept on July 8, 2018
 */

#ifndef _SOCKET_PROBE_H_
#define _SOCKET_PROBE_H_

#include <vector>
#include "basic_socket.h"
#include "SocketPollTable.h"

namespace Impact {
	class SocketProbe {
		SocketProbe();
	public:
	  static int select(
		std::vector<basic_socket*> readHandles,
		std::vector<basic_socket*> writeHandles,
		int timeout=-1, unsigned int microTimeout=0)
		/* throw(std::runtime_error) */;

	  static int poll(SocketPollTable& token, int timeout)
		/* throw(std::runtime_error) */;
	};
}

#endif
