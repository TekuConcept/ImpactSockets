/**
 * Created by TekuConcept on July 11, 2018
 */

#include "sockets/async_pipeline.h"

#include <csignal>

#include "sockets/environment.h"
#include "sockets/impact_error.h"

using namespace impact;
using namespace internal;

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
	//std::signal(SIGPIPE, SIG_IGN);
	// lazy startup thread
}


async_pipeline::~async_pipeline()
{
	// shutdown thread
}


int
async_pipeline::_M_create_pollfd(int __descriptor)
{
	struct pollfd token;
	token.fd      = __descriptor;
	token.events  = 0;
	token.revents = 0;
	m_handles_.push_back(token);
	return m_handles_.size() - 1;
}


std::future<int>
async_pipeline::_M_enqueue(
	int                         __descriptor,
	ioaction                    __ioaction,
	const std::function<int()>& __iofunction)
{
	if (__descriptor < 0)
		throw impact_error("Invalid socket descriptor");

	// NOTE: map[index] will either find if exists or create otherwise
	auto& info            = m_info_[__descriptor];
	if (info.pollfd_index == (size_t)(-1))
		info.pollfd_index = _M_create_pollfd(__descriptor);
	auto& token           = m_handles_[info.pollfd_index];

	short event;
	struct action_info* action;
	switch (__ioaction) {
	case SEND:
	case SENDTO: {
		action = &info.input;
		event  = (short)poll_flags::OUT;
		break;
	}
	case RECV:
	case RECVFROM:
	case ACCEPT: {
		action = &info.output;
		event  = (short)poll_flags::IN;
		break;
	}
	}

	if (action->state != action_state::FREE)
		throw impact_error("Action still pending");

	action->state      = action_state::PENDING;
	// action->promise should already be initialized
	// either via the ctor() or reset-when-resolved
	action->iofunction = __iofunction;
	token.events      |= event;

	// NOTE: compile with -pthread in NIX to avoid std::exception
	return action->promise.get_future();
}


std::future<int>
async_pipeline::send(
	basic_socket* __socket,
	const void*   __buffer,
	int           __length,
	message_flags __flags)
{
	if (!__socket)
		throw impact_error("Socket argument NULL");

	CATCH_ASSERT(
		auto iofunction = std::bind(
			&basic_socket::send,
			__socket,
			__buffer,
			__length,
			__flags
		);
		return _M_enqueue(__socket->get(), ioaction::SEND, iofunction);
	);
}


std::future<int>
async_pipeline::sendto(
	basic_socket*      __socket,
	const void*        __buffer,
	int                __length,
	unsigned short     __port,
	const std::string& __address,
	message_flags      __flags)
{
	if (!__socket)
		throw impact_error("Socket argument NULL");

	CATCH_ASSERT(
		auto iofunction = std::bind(
			&basic_socket::sendto,
			__socket,
			__buffer,
			__length,
			__port,
			__address,
			__flags
		);
		return _M_enqueue(__socket->get(), ioaction::SENDTO, iofunction);
	)
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

	CATCH_ASSERT(
		auto iofunction = std::bind(
			&basic_socket::recv,
			__socket,
			__buffer,
			__length,
			__flags
		);
		return _M_enqueue(__socket->get(), ioaction::RECV, iofunction);
	)
}


std::future<int>
async_pipeline::recvfrom(
	basic_socket*   __socket,
	void*           __buffer,
	int             __length,
	unsigned short* __port,
	std::string*    __address,
	message_flags   __flags
	)
{
	if (!__socket)
		throw impact_error("Socket argument NULL");

	CATCH_ASSERT(
		auto iofunction = std::bind(
			&basic_socket::recvfrom,
			__socket,
			__buffer,
			__length,
			__port,
			__address,
			__flags
		);
		return _M_enqueue(__socket->get(), ioaction::RECVFROM, iofunction);
	)
}


std::future<int>
async_pipeline::accept(
	basic_socket* __socket,
	basic_socket* __client)
{
	if (!(__socket && __client))
		throw impact_error("One or more NULL arguments");

	CATCH_ASSERT(
		auto iofunction = [&]() -> int {
			DEREF(__client) = __socket->accept();
			return 0;
		};
		return _M_enqueue(__socket->get(), ioaction::ACCEPT, iofunction);
	)
}
