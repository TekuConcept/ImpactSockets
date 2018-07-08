/**
 * Created by TekuConcept on July 8, 2018
 */

#include "Environment.h"
#include "Generic.h"
#include "SocketProbe.h"
#include "io_error.h"
#include <string>

#if defined(__WINDOWS__)
	#define SOC_POLL WSAPoll
#else
	#define SOC_POLL ::poll
#endif

#define ASSERT(title,cond)\
 	if (cond) {\
 		std::string message( title );\
 		message.append(Internal::getErrorMessage());\
 		throw io_error(message);\
 	}

using namespace Impact;


int SocketProbe::select(
	std::vector<basic_socket*> readHandles,
	std::vector<basic_socket*> writeHandles,
	int timeout, unsigned int microTimeout) {
	struct timeval time_s;
	time_s.tv_sec = (unsigned int)(0xFFFFFFFF&timeout);
	time_s.tv_usec = microTimeout;

	fd_set readSet, writeSet;
	unsigned int nfds = 0;

	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);

	for(const auto& handle : readHandles) {
		unsigned int descriptor = handle->get();
		FD_SET(descriptor, &readSet);
		if (descriptor > nfds) nfds = descriptor;
	}

	for(const auto& handle : writeHandles) {
		unsigned int descriptor = handle->get();
		FD_SET(descriptor, &writeSet);
		if (descriptor > nfds) nfds = descriptor;
	}

	auto status = ::select(nfds + 1, &readSet, &writeSet, NULL,
		((timeout<0)?NULL:&time_s));

	ASSERT("SocketProbe::select()\n", status == SOCKET_ERROR);

	return status;
}


int SocketProbe::poll(SocketPollTable& token, int timeout) {
	/* timeout: -1 blocking, 0 nonblocking, 0> timeout */
	struct pollfd* fds = token._descriptors_.data();
	auto size = token.size();
	auto status = SOC_POLL(fds, size, timeout);

	ASSERT("SocketProbe::poll()\n", status == SOCKET_ERROR);

	/* status: -1 error, 0 timeout, 0> success */
	return status;
}
