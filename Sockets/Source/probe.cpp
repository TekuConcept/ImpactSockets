/**
 * Created by TekuConcept on July 8, 2018
 */

#include "sockets/probe.h"

#include <string>

#include "sockets/environment.h"
#include "sockets/generic.h"
#include "sockets/io_error.h"

#if defined(__APPLE__)
	#include <unistd.h> // select()
#endif

#if defined(__WINDOWS__)
	#define POLL WSAPoll
#else
	#define POLL ::poll
	#define SOCKET_ERROR -1
#endif

#define ASSERT(title,cond)\
 	if (cond) {\
 		std::string message( title );\
 		message.append(internal::error_message());\
 		throw io_error(message);\
 	}

using namespace impact;


int
probe::select(
	std::vector<basic_socket*> __read_handles,
	std::vector<basic_socket*> __write_handles,
	int                        __timeout,
	unsigned int               __micro_timeout)
{
	struct timeval time_s;
	time_s.tv_sec  = (unsigned int)(0xFFFFFFFF&__timeout);
	time_s.tv_usec = __micro_timeout;

	fd_set read_set, write_set;
	unsigned int num_fds = 0;

	FD_ZERO(&read_set);
	FD_ZERO(&write_set);

	for (const auto& handle : __read_handles) {
		unsigned int descriptor = handle->get();
		FD_SET(descriptor, &read_set);
		if (descriptor > num_fds)
			num_fds = descriptor;
	}

	for (const auto& handle : __write_handles) {
		unsigned int descriptor = handle->get();
		FD_SET(descriptor, &write_set);
		if (descriptor > num_fds)
			num_fds = descriptor;
	}

	auto status = ::select(
		num_fds + 1,
		&read_set,
		&write_set,
		NULL,
		((__timeout<0)?NULL:&time_s)
	);

	ASSERT("probe::select()\n", status == SOCKET_ERROR);
	return status;
}


int
probe::poll(
	poll_vector& __token,
	int          __timeout)
{
	/* timeout: -1 blocking, 0 nonblocking, 0> timeout */
	struct pollfd* token = __token.m_descriptors_.data();
	auto size            = __token.size();
	auto status          = POLL(token, size, __timeout);

	ASSERT("probe::poll()\n", status == SOCKET_ERROR);
	/* status: -1 error, 0 timeout, 0> success */
	return status;
}
