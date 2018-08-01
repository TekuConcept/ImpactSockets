/**
 * Created by TekuConcept on July 11, 2018
 */

#include "sockets/async_pipeline.h"

#include <cstdlib>
#include <algorithm>
#include <chrono>

#include "utils/environment.h"
#include "sockets/impact_error.h"
#include "sockets/types.h"
#include "sockets/generic.h"

using namespace impact;
using namespace internal;

#include <iostream>
#define VERB(x) std::cout << x << std::endl


async_pipeline&
async_pipeline::instance()
{
  static async_pipeline unit;
  return unit;
}


async_pipeline::async_pipeline()
{
	m_poll_granularity_ = k_default_granularity_;
    m_thread_ready_     = false;
	m_thread_closing_   = false;
	m_thread_has_work_  = false;
	m_thread_pending_   = 0;
	// m_main_ready_       = false;

	_M_begin();
}


async_pipeline::~async_pipeline()
{
	_M_end();
}


void
async_pipeline::granularity(int __milliseconds)
{
	if (__milliseconds < 0)
		m_poll_granularity_ = k_default_granularity_;
	else m_poll_granularity_ = __milliseconds;
}


void
async_pipeline::add_object(
	int              __socket,
	async_object_ptr __object)
{
	if (__socket < 0)
		throw impact_error("Invalid socket");
	
	{
		std::lock_guard<std::mutex> lock(m_work_mtx_);
		m_work_pending_.push_back(handle_info(__socket,__object));
	}
	_M_notify_pending(1);
}


void
async_pipeline::remove_object(int __socket)
{
	if (__socket < 0)
		throw impact_error("Invalid socket");
	
	{
		std::lock_guard<std::mutex> lock(m_work_mtx_);
		m_work_removed_.push_back(__socket);
	}
	_M_notify_pending(1);
}


void
async_pipeline::_M_notify_pending(int p)
{
	{
		std::lock_guard<std::mutex> lock(m_thread_mtx_);
		m_thread_pending_ += p;
	}
	m_thread_cv_.notify_one();
}


void
async_pipeline::notify()
{
	{
		std::lock_guard<std::mutex> lock(m_thread_mtx_);
		m_thread_has_work_ = true;
	}
	m_thread_cv_.notify_one();
}


void
async_pipeline::_M_copy_pending_to_queue()
{
	for (auto& token : m_work_pending_) {
		auto target_info = m_work_info_.find(token.first);
		if (target_info == m_work_info_.end()) {
			m_work_info_[token.first] = token.second;
			struct poll_handle handle;
			handle.socket = token.first;
			handle.events = (short)poll_flags::IN;
			m_work_handles_.push_back(handle);
		}
		else
			target_info->second = token.second;
	}
	m_work_pending_.clear();
}


void
async_pipeline::_M_remove_pending_from_queue()
{
	for (auto& token : m_work_removed_) {
		auto handle = std::find_if(
			m_work_handles_.begin(),
			m_work_handles_.end(),
			[&](const poll_handle& __handle) -> bool {
				return std::abs(__handle.socket) == token;
			}
		);
		if (handle != m_work_handles_.end()) {
			// for every handle there is associated info - remove both
			m_work_info_.erase(m_work_info_.find(std::abs(handle->socket)));
			m_work_handles_.erase(handle);
		}
	}
	m_work_removed_.clear();
}


bool
async_pipeline::_M_update_handles()
{
	size_t ignored = 0;
	auto error = (socket_error)error_code();
	for (size_t i = 0; /* (!m_thread_closing_) && */
		(i < m_work_handles_.size()); i++) {
		auto& key   = m_work_handles_[i];
		auto socket = std::abs(key.socket);
		auto value  = m_work_info_.find(socket);
		
		auto option = value->second->async_callback(&key,error);
		
		switch (option) {
		case async_option::IGNORE:
			socket            = -socket;
			/* special 0-fd case */
			if (socket == 0) ignored++;
		case async_option::CONTINUE:
			key.return_events = 0;
			key.socket        = socket;
			if (socket < 0) ignored++;
			break;
		default: /* async_option::QUIT */
			m_work_handles_.erase(m_work_handles_.begin()+i);
			m_work_info_.erase(value);
			i--;
			break;
		}
	}
	return ignored == m_work_handles_.size();
}


void
async_pipeline::_M_dowork()
{
	int pending_satisfied = 0;
	{ /* worker locked scope */
		std::lock_guard<std::mutex> lock(m_work_mtx_);
		pending_satisfied += m_work_pending_.size();
		pending_satisfied += m_work_removed_.size();
		_M_copy_pending_to_queue();
		_M_remove_pending_from_queue();
	} /* end locked scope */

	auto status = poll(&m_work_handles_, (int)m_poll_granularity_);
	UNUSED(status);
	
	auto has_work = !_M_update_handles();
	
	{ /* thread locked scope */
		std::lock_guard<std::mutex> lock(m_thread_mtx_);
		m_thread_has_work_ = has_work; // <- *
		m_thread_pending_ -= pending_satisfied;
	} /* end locked scope */
}


void
async_pipeline::_M_begin()
{   
    m_thread_ = std::thread([&](){
    	do {
        	std::unique_lock<std::mutex> lock(m_thread_mtx_);
            m_thread_cv_.wait(lock, [&]() -> bool {
                return
                	m_thread_closing_  ||
                	m_thread_has_work_ ||
                	(m_thread_pending_ > 0);
            });
            if (m_thread_closing_) break;
            lock.unlock(); // release the lock
            
            _M_dowork();
        } while (true);
    });
}


void
async_pipeline::_M_end()
{
	{
		std::lock_guard<std::mutex> lock(m_thread_mtx_);
		m_thread_closing_ = true;
	}
	m_thread_cv_.notify_all();
    if (m_thread_.joinable())
        m_thread_.join();
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
|  Async Object Function Implementations                                      |
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


async_functor::async_functor(
	std::function<async_option(poll_handle*,socket_error)> __callback)
: m_callback_(__callback)
{}


async_functor::~async_functor()
{}


async_option
async_functor::async_callback(
	poll_handle* __handle,
	socket_error __error)
{
	if (m_callback_)
		return m_callback_(__handle, __error);
	return async_option::QUIT;
}
