/**
 * Created by TekuConcept on July 8, 2018
 */

#include "sockets/probe.h"

#include <string>

#include "utils/environment.h"
#include "sockets/generic.h"
#include "sockets/impact_error.h"

#if !defined(__WINDOWS__)
	#include <sys/poll.h> // For struct pollfd, poll()
#endif

#if defined(__APPLE__)
	#include <unistd.h> // select()
#endif

#if defined(__WINDOWS__)
	#define POLL WSAPoll
#else
	#define POLL ::poll
	#define SOCKET_ERROR -1
#endif

#define ASSERT(cond)\
 	if (!(cond)) throw impact_error(internal::error_message());

int
impact::select(
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

	return status;
}


impact::poll_handle::poll_handle()
: socket(-1), events(0), return_events(0)
{}


int
impact::poll(
	std::vector<impact::poll_handle>* __handles,
	int                               __timeout)
{
	if (!__handles)
		return 0;

	/* timeout: -1 blocking, 0 nonblocking, 0> timeout */
	struct poll_handle* handles     = &(*__handles)[0];
	struct pollfd* poll_descriptors = reinterpret_cast<struct pollfd*>(handles);
	auto size                       = __handles->size();
	auto status                     = POLL(poll_descriptors, size, __timeout);

	/* status: -1 error, 0 timeout, 0 > success */
	return status;
}
