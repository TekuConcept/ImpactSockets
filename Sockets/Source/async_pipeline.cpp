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
	#define POLL WSAPoll
#else
	#define CCHAR_PTR void*
	#define CHAR_PTR void*
	#define SOCKET_ERROR -1
	#define INVALID_SOCKET -1
	#define POLL ::poll
#endif

#define ASSERT(cond)\
 	if (!(cond)) throw impact_error(internal::error_message());

#define CATCH_ASSERT(code)\
	try { code }\
	catch (impact_error e) { throw; }\
	catch (...) { throw impact_error("Unknown internal error"); }


async_pipeline&
async_pipeline::instance()
{
  static async_pipeline unit;
  return unit;
}


async_pipeline::async_pipeline()
: m_granularity_(50)
{
#if !defined(__WINDOWS__)
	// this should already be done by basic_socket()
	// but run it just in case
	no_sigpipe();
#endif
}


async_pipeline::~async_pipeline()
{}


void
async_pipeline::granularity(int __milliseconds)
{
	m_granularity_ = __milliseconds;
}


bool
async_pipeline::_M_has_work()
{
	std::lock_guard<std::mutex> lock(m_var_mtx_);
	return m_pending_.size() > 0 || m_handles_.size() > 0;
}


void
async_pipeline::_M_dowork()
{
	do {
		_M_copy_pending_to_queue();
		
		auto status = POLL(
			&m_handles_[0],
			m_handles_.size(),
			(int)m_granularity_
		);
		
		if (status > 0) _M_process_events();
		else if (status < 0) _M_recover_fetal(error_message());
	} while(_M_has_work());
}


// void
// async_pipeline::_M_enqueue(pending_handle* __handle)
// {
// 	std::lock_guard<std::mutex> lock(m_var_mtx_);
// 	m_pending_.push_back(std::move(*__handle));
// 	m_thread_cv_.notify_one();
// }


void
async_pipeline::_M_copy_pending_to_queue()
{
	std::lock_guard<std::mutex> lock(m_var_mtx_);
	// if (m_pending_.size() == 0)
	// 	return;

	// for (auto& handle : m_pending_) {
	// 	// NOTE: map[index] will either find or create
	// 	auto& info            = m_info_[handle.descriptor];
	// 	if (info.pollfd_index == (size_t)(-1))
	// 		info.pollfd_index = _M_create_pollfd(handle.descriptor);
	// 	auto& pollfd_handle   = m_handles_[info.pollfd_index];

	// 	short event;
	// 	struct action_info* action;
	// 	switch (handle.action) {
	// 	case SEND:
	// 	case SENDTO: {
	// 		action = &info.output;
	// 		event  = (short)poll_flags::OUT;
	// 		break;
	// 	}
	// 	case RECV:
	// 	case RECVFROM:
	// 	case ACCEPT: {
	// 		action = &info.input;
	// 		event  = (short)poll_flags::IN;
	// 		break;
	// 	}}

	// 	if (action->state != action_state::FREE) {
	// 		try {
	// 			throw impact_error("Action still pending");
	// 		} catch (...) {
	// 			handle.promise.set_exception(std::current_exception());
	// 		}
	// 		continue;
	// 	}

	// 	action->state         = action_state::PENDING;
	// 	action->callback      = handle.callback;
	// 	std::swap(handle.promise, action->promise);
	// 	pollfd_handle.events |= event;
	// }
	
	// m_pending_.clear();
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


void
async_pipeline::_M_recover_fetal(const std::string& __error_message)
{
	std::string message("Request canceled (pipeline error)\n");
	message.append(__error_message);
	m_handles_.clear();
}


void
async_pipeline::_M_process_events()
{
	// m_handles_ have return event information
	/*
	info = m_info_[handle.fd];
	handle.revents & (int)poll_flags::IN:
		auto status = info.input.callback()
		info.input.promise.set_value(status)
	handle.revents & (int)poll_flags::OUT:
		auto status = info.output.callback()
		info.output.promise.set_value(status)
	handle.revents & (int)poll_flags::HANGUP:
		set input/output promise values as needed
		schedule handle removal
	handle.revents & (int)poll_flags::INVALID ||
	handle.revents & (int)poll_flags::ERROR:
		set input/output promise exceptions as needed
		schedule handle removal
	*/
}
