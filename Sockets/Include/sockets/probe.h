/**
 * Created by TekuConcept on July 8, 2018
 */

#ifndef _IMPACT_PROBE_H_
#define _IMPACT_PROBE_H_

#include <vector>

#include "sockets/basic_socket.h"
#include "sockets/poll_vector.h"

namespace impact {
	class probe {
	public:
	  static int select(
			std::vector<basic_socket*> readHandles,
			std::vector<basic_socket*> writeHandles,
			int timeout=-1, unsigned int microTimeout=0)
			/* throw(std::runtime_error) */;

	  static int poll(poll_vector& token, int timeout)
			/* throw(std::runtime_error) */;

	private:
		probe();
	};
}

#endif
