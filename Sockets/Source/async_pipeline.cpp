/**
 * Created by TekuConcept on July 11, 2018
 */

#include "sockets/async_pipeline.h"

#include <algorithm>

#include "sockets/environment.h"
#include "sockets/impact_error.h"
#include "sockets/types.h"
#include "sockets/generic.h"

using namespace impact;
using namespace internal;

async_functor::async_functor(
	std::function<void(poll_handle*,socket_error)> __callback)
: m_callback_(__callback)
{}


async_functor::~async_functor()
{}


void
async_functor::async_callback(
	poll_handle* __handle,
	socket_error __error)
{
	if (m_callback_)
		m_callback_(__handle, __error);
}


async_pipeline&
async_pipeline::instance()
{
  static async_pipeline unit;
  return unit;
}


async_pipeline::async_pipeline()
: m_granularity_(50)
{}


async_pipeline::~async_pipeline()
{
	/* prevent shutdown segfaults */
	std::lock_guard<std::mutex> lock();
	m_pending_add_.clear();
	m_pending_remove_.clear();
	m_handles_.clear();
	m_info_.clear();
}


void
async_pipeline::granularity(int __milliseconds)
{
	m_granularity_ = __milliseconds;
}


void
async_pipeline::add_object(
	const basic_socket* __socket,
	async_object_ptr    __object)
{
	if (!__socket && !(*__socket))
		throw impact_error("Invalid socket");
	
	std::lock_guard<std::mutex> lock(m_var_mtx_);
	m_pending_add_.push_back(handle_info(__socket->get(),__object));
	_M_notify_one();
}


void
async_pipeline::remove_object(const basic_socket* __socket)
{
	if (!__socket && !(*__socket))
		throw impact_error("Invalid socket");
	
	std::lock_guard<std::mutex> lock(m_var_mtx_);
	m_pending_remove_.push_back(__socket->get());
	_M_notify_one();
}


bool
async_pipeline::_M_has_work()
{
	std::lock_guard<std::mutex> lock(m_var_mtx_);
	return (m_handles_.size() > 0) || (
		(m_pending_add_.size() > 0) ||
		(m_pending_remove_.size() > 0)
	);
}


void
async_pipeline::_M_dowork()
{
	do {
		{ /* locked scope */
			std::lock_guard<std::mutex> lock(m_var_mtx_);
			_M_copy_pending_to_queue();
			_M_remove_pending_from_queue();
		} /* end locked scope */
		
		auto status = poll(&m_handles_, (int)m_granularity_);
		UNUSED(status);
		
		auto error = (socket_error)error_code();
		for (auto& handle : m_handles_) {
			m_info_[handle.socket]->async_callback(&handle,error);
			handle.return_events = 0;
		}
	} while(_M_has_work());
}


void
async_pipeline::_M_copy_pending_to_queue()
{
	for (auto& token : m_pending_add_) {
		auto target_info = m_info_.find(token.first);
		if (target_info == m_info_.end()) {
			m_info_[token.first] = token.second;
			struct poll_handle handle;
			handle.socket = token.first;
			handle.events = (short)poll_flags::IN;
			m_handles_.push_back(handle);
		}
		else
			target_info->second = token.second;
	}
	m_pending_add_.clear();
}


void
async_pipeline::_M_remove_pending_from_queue()
{
	for (auto& token : m_pending_remove_) {
		auto handle = std::find_if(
			m_handles_.begin(),
			m_handles_.end(),
			[&](const poll_handle& __handle) -> bool {
				return __handle.socket == token;
			}
		);
		if (handle != m_handles_.end()) {
			// for every handle there is associated info - remove both
			m_info_.erase(m_info_.find(handle->socket));
			m_handles_.erase(handle);
		}
	}
	m_pending_remove_.clear();
}
