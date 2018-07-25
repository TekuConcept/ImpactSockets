/**
 * Created by TekuConcept on July 11, 2018
 */

#include "sockets/async_pipeline.h"

#include <csignal>

#include "sockets/environment.h"
#include "sockets/impact_error.h"
#include "sockets/generic.h"

using namespace impact;
using namespace internal;

#if defined(__WINDOWS__)
	#define CCHAR_PTR const char*
	#define CHAR_PTR char*
#else
	#define CCHAR_PTR void*
	#define CHAR_PTR void*
	#define SOCKET_ERROR -1
	#define INVALID_SOCKET -1
#endif

#define ASSERT(cond)\
 	if (!(cond)) throw impact_error(internal::error_message());

#define CATCH_ASSERT(code)\
	try { code }\
	catch (impact_error e) { throw; }\
	catch (...) { throw impact_error("Unknown internal error"); }

async_pipeline::action_info::action_info()
: state(action_state::FREE)
{}


async_pipeline::handle_info::handle_info()
: pollfd_index((size_t)(-1))
{}


async_pipeline&
async_pipeline::instance()
{
  static async_pipeline unit;
  return unit;
}


async_pipeline::async_pipeline()
{
#if !defined(__WINDOWS__)
	// this should already be done by basic_socket()
	// but run it just in case
	no_sigpipe();
#endif
}


async_pipeline::~async_pipeline()
{}


bool
async_pipeline::_M_has_work()
{
	std::lock_guard<std::mutex> lock(m_var_mtx_);
	return m_pending_.size() > 0 || m_handles_.size() > 0;
}


void
async_pipeline::_M_dowork()
{
	/* TODO
	// LOOP:
	_M_copy_pending_to_queue();
	if (m_handles_.size() == 0)
		return;
	
	auto status = POLL(&m_handles_[0], m_handles_.size(), 0);
	if (status > 0) {
		// iterate through handles and process
		for (const auto& handle : m_handles_) {
			auto info = m_info_[handle.fd];
			if (handle.revents & (int)poll_flags::IN) {
				try {
					auto status = info.input.callback();
					info.input.promise.set_value(status);
				}
				catch (...) {
					info.input.promise.set_exception(std::current_exception());
				}
			}
			if (handle.revents & (int)poll_flags::OUT) {
				try {
					auto status = info.output.callback();
					info.output.promise.set_value(status);
				}
				catch (...) {
					info.output.promise.set_exception(std::current_exception());
				}
			}
			// handle POLLERR, POLLHUP, & POLLNVAL
		}
	}
	else if (status == 0) {
		// no handles to update - continue
	}
	else {
		// poll error - try recovering
		// 1. set_exception on all handles
		// 2. clear handles and handle info
	}
	*/
}


void
async_pipeline::_M_enqueue(pending_handle* __handle)
{
	std::lock_guard<std::mutex> lock(m_var_mtx_);
	m_pending_.push_back(std::move(*__handle));
	m_thread_cv_.notify_one();
}


// int
// async_pipeline::_M_create_pollfd(int __descriptor)
// {
// 	struct pollfd token;
// 	token.fd      = __descriptor;
// 	token.events  = 0;
// 	token.revents = 0;
// 	m_handles_.push_back(token);
// 	return m_handles_.size() - 1;
// }


// std::future<int>
// async_pipeline::_M_enqueue(
// 	int                         __descriptor,
// 	ioaction                    __ioaction,
// 	const std::function<int()>& __iofunction)
// {
// 	if (__descriptor < 0)
// 		throw impact_error("Invalid socket descriptor");

// 	// NOTE: map[index] will either find if exists or create otherwise
// 	auto& info            = m_info_[__descriptor];
// 	if (info.pollfd_index == (size_t)(-1))
// 		info.pollfd_index = _M_create_pollfd(__descriptor);
// 	auto& pollfd_handle   = m_handles_[info.pollfd_index];

// 	short event;
// 	struct action_info* action;
// 	switch (__ioaction) {
// 	case SEND:
// 	case SENDTO: {
// 		action = &info.input;
// 		event  = (short)poll_flags::OUT;
// 		break;
// 	}
// 	case RECV:
// 	case RECVFROM:
// 	case ACCEPT: {
// 		action = &info.output;
// 		event  = (short)poll_flags::IN;
// 		break;
// 	}
// 	}

// 	if (action->state != action_state::FREE)
// 		throw impact_error("Action still pending");

// 	action->state         = action_state::PENDING;
// 	// action->promise should already be initialized
// 	// either via the ctor() or reset-when-resolved
// 	action->iofunction    = __iofunction;
// 	pollfd_handle.events |= event;

// 	// NOTE: compile with -pthread in NIX to avoid std::exception
// 	return action->promise.get_future();
// }


std::future<int>
async_pipeline::send(
	basic_socket* __socket,
	const void*   __buffer,
	int           __length,
	message_flags __flags)
{
	if (!__socket)
		throw impact_error("Socket argument NULL");
	auto descriptor   = __socket->get(); // force copy

	struct pending_handle handle;
	handle.descriptor = descriptor;
	handle.action     = ioaction::SEND;
	handle.callback   = [&]() -> int {
		auto status   = ::send(
			descriptor,
			(CCHAR_PTR)__buffer,
			__length,
			(int)__flags
		);
		ASSERT(status != SOCKET_ERROR);
		return status;
	};
	auto future = handle.promise.get_future();
	
	_M_enqueue(&handle);
	return future;
}


std::future<int>
async_pipeline::sendto(
	basic_socket*  __socket,
	const void*    __buffer,
	int            __length,
	unsigned short __port,
	std::string    __address,
	message_flags  __flags)
{
	if (!__socket)
		throw impact_error("Socket argument NULL");
	// force copy details
	auto descriptor   = __socket->get();
	auto domain       = __socket->domain();
	auto type         = __socket->type();
	auto protocol     = __socket->protocol();
	
	struct pending_handle handle;
	handle.descriptor = descriptor;
	handle.action     = ioaction::SENDTO;
	handle.callback   = [&]() -> int {
		struct sockaddr_in destination_address;
		CATCH_ASSERT(
			internal::fill_address(
				domain, type, protocol,
				__address,
				__port,
				destination_address
			);
		)
		auto status = ::sendto(
			descriptor,
			(CCHAR_PTR)__buffer,
			__length,
			(int)__flags,
			(struct sockaddr*)&destination_address,
			sizeof(destination_address)
		);
		ASSERT(status != SOCKET_ERROR)
		return status;
	};
	auto future = handle.promise.get_future();
	
	_M_enqueue(&handle);
	return future;
}


std::future<int>
async_pipeline::recv(
	basic_socket* __socket,
	void*         __buffer,
	int           __length,
	message_flags __flags)
{
	if (!__socket)
		throw impact_error("Socket argument NULL");
	auto descriptor   = __socket->get(); // force copy
	
	struct pending_handle handle;
	handle.descriptor = descriptor;
	handle.action     = ioaction::RECV;
	handle.callback   = [&]() -> int {
		int status = ::recv(
			descriptor,
			(CHAR_PTR)__buffer,
			__length,
			(int)__flags
		);
		ASSERT(status != SOCKET_ERROR)
		return status; /* number of bytes received or EOF */
	};
	auto future = handle.promise.get_future();
	
	_M_enqueue(&handle);
	return future;
}


// NOTE: shared_ptr used to prevent writing to freed memory (SIGSEGV)
std::future<int>
async_pipeline::recvfrom(
	basic_socket*                   __socket,
	void*                           __buffer,
	int                             __length,
	std::shared_ptr<unsigned short> __port,
	std::shared_ptr<std::string>    __address,
	message_flags                   __flags
	)
{
	if (!__socket)
		throw impact_error("Socket argument NULL");
	auto descriptor   = __socket->get(); // force copy
	
	struct pending_handle handle;
	handle.descriptor = descriptor;
	handle.action     = ioaction::RECVFROM;
	handle.callback   = [&]() -> int {
		struct sockaddr_in client_address;
		socklen_t address_length = sizeof(client_address);
		auto status = ::recvfrom(
			descriptor,
			(CHAR_PTR)__buffer,
			__length,
			(int)__flags,
			(struct sockaddr*)&client_address,
			(socklen_t*)&address_length
		);
		ASSERT(status != SOCKET_ERROR)
		if (__address)
			*__address = inet_ntoa(client_address.sin_addr);
		if (__port)
			*__port    = ntohs(client_address.sin_port);
		return status;
	};
	auto future = handle.promise.get_future();
	
	_M_enqueue(&handle);
	return future;
}


std::future<int>
async_pipeline::accept(
	basic_socket* __socket,
	basic_socket* __client)
{
	if (!(__socket && __client))
		throw impact_error("One or more NULL arguments");
	// force copy details
	auto descriptor   = __socket->get();
	auto domain       = __socket->domain();
	auto type         = __socket->type();
	auto protocol     = __socket->protocol();
	
	struct pending_handle handle;
	handle.descriptor = descriptor;
	handle.action     = ioaction::ACCEPT;
	handle.callback   = [&]() -> int {
		basic_socket peer;
		peer.m_info_->descriptor = ::accept(descriptor, NULL, NULL);
		ASSERT(peer.m_info_->descriptor != INVALID_SOCKET)
		peer.m_info_->ref_count  = 1;
		peer.m_info_->wsa        = false;
		peer.m_info_->domain     = domain;
		peer.m_info_->type       = type;
		peer.m_info_->protocol   = protocol;
		DEREF(__client) = std::move(peer);
		return 0;
	};
	auto future = handle.promise.get_future();
	
	_M_enqueue(&handle);
	return future;
}
