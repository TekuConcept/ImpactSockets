/**
 * Created by TekuConcept on July 11, 2018
 */

#include "sockets/async_pipeline.h"

#include <cstdlib>
#include <algorithm>

#include "sockets/environment.h"
#include "sockets/impact_error.h"
#include "sockets/types.h"
#include "sockets/generic.h"

using namespace impact;
using namespace internal;

#include <iostream>
#define VERB(x) std::cout << x << std::endl

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


async_pipeline&
async_pipeline::instance()
{
  static async_pipeline unit;
  return unit;
}


async_pipeline::async_pipeline()
{
	m_var_mtx_        = std::make_shared<type_object<std::mutex>>();
	m_handles_        = std::make_shared<type_object<std::vector<poll_handle>>>();
	m_info_           = std::make_shared<type_object<std::map<int, async_object_ptr>>>();
	m_pending_add_    = std::make_shared<type_object<std::vector<handle_info>>>();
	m_pending_remove_ = std::make_shared<type_object<std::vector<int>>>();
	m_shutting_down_  = std::make_shared<type_object<std::atomic<bool>>>();
	m_granularity_    = std::make_shared<type_object<std::atomic<int>>>();

	_M_register_obj(m_var_mtx_);
	_M_register_obj(m_handles_);
	_M_register_obj(m_info_);
	_M_register_obj(m_pending_add_);
	_M_register_obj(m_pending_remove_);
	_M_register_obj(m_shutting_down_);
	_M_register_obj(m_granularity_);
	
	m_granularity_->type   = 50;
	m_shutting_down_->type = false;
	m_has_work_            = false;
}


async_pipeline::~async_pipeline()
{
	m_shutting_down_->type = true;
	m_has_work_            = false;
}


void
async_pipeline::granularity(int __milliseconds)
{
	m_granularity_->type = __milliseconds;
}


void
async_pipeline::add_object(
	int              __socket,
	async_object_ptr __object)
{
	if (__socket < 0)
		throw impact_error("Invalid socket");
	
	std::lock_guard<std::mutex> lock(m_var_mtx_->type);
	m_pending_add_->type.push_back(handle_info(__socket,__object));
	m_has_work_ = true;
	_M_notify_one();
}


void
async_pipeline::remove_object(int __socket)
{
	if (__socket < 0)
		throw impact_error("Invalid socket");
	
	std::lock_guard<std::mutex> lock(m_var_mtx_->type);
	m_pending_remove_->type.push_back(__socket);
	m_has_work_ = true;
	_M_notify_one();
}


void
async_pipeline::notify()
{
	m_has_work_ = true;
	_M_notify_one();
}


bool
async_pipeline::_M_has_work()
{
	return m_has_work_ > 0;
}


void
async_pipeline::_M_dowork()
{
	do {
		{ /* locked scope */
			if (m_var_mtx_->type.try_lock()) {
				_M_copy_pending_to_queue(&m_handles_->type, &m_info_->type);
				_M_remove_pending_from_queue(&m_handles_->type, &m_info_->type);
				m_has_work_ = false;
				m_var_mtx_->type.unlock();
			}
			else continue;
		} /* end locked scope */
		
		auto status = poll(&m_handles_->type, (int)m_granularity_->type);
		UNUSED(status);
		
		bool sleep = _M_update_handles();
		if (sleep) return;
	} while(!m_shutting_down_->type);
}


void
async_pipeline::_M_copy_pending_to_queue(
	std::vector<poll_handle>*        __handles,
	std::map<int, async_object_ptr>* __info)
{
	for (auto& token : m_pending_add_->type) {
		auto target_info = __info->find(token.first);
		if (target_info == __info->end()) {
			(*__info)[token.first] = token.second;
			struct poll_handle handle;
			handle.socket = token.first;
			handle.events = (short)poll_flags::IN;
			__handles->push_back(handle);
		}
		else
			target_info->second = token.second;
	}
	m_pending_add_->type.clear();
}


void
async_pipeline::_M_remove_pending_from_queue(
	std::vector<poll_handle>*        __handles,
	std::map<int, async_object_ptr>* __info)
{
	for (auto& token : m_pending_remove_->type) {
		auto handle = std::find_if(
			__handles->begin(),
			__handles->end(),
			[&](const poll_handle& __handle) -> bool {
				return __handle.socket == token;
			}
		);
		if (handle != __handles->end()) {
			// for every handle there is associated info - remove both
			__info->erase(__info->find(handle->socket));
			__handles->erase(handle);
		}
	}
	m_pending_remove_->type.clear();
}


bool
async_pipeline::_M_update_handles()
{
	size_t ignored = 0;
	auto error = (socket_error)error_code();
	for (size_t i = 0; /* (!m_shutting_down_->type) && */
		(i < m_handles_->type.size()); i++) {
		auto& key   = m_handles_->type[i];
		auto socket = std::abs(key.socket);
		auto value  = m_info_->type.find(socket);
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
			m_handles_->type.erase(m_handles_->type.begin()+i);
			m_info_->type.erase(value);
			i--;
			break;
		}
	}
	return ignored == m_handles_->type.size();
}
