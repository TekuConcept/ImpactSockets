/**
 * Created by TekuConcept on June 19, 2018
 */

#include "sockets/poll_vector.h"

using namespace impact;

poll_vector::poll_vector()
{}


poll_vector::poll_vector(poll_initializer __handles)
{
	for (auto& handle : __handles)
		push_back(handle);
}


poll_vector::~poll_vector()
{}


void
poll_vector::reset_events()
{
	std::lock_guard<std::mutex> lock(m_mtx_);
	for(auto& descriptor : m_descriptors_)
		descriptor.revents = 0;
}


bool
poll_vector::empty() const
{
	return m_descriptors_.empty();
}


unsigned int
poll_vector::size() const
{
	return m_descriptors_.size();
}


void
poll_vector::reserve(unsigned int __capacity)
{
	std::lock_guard<std::mutex> lock(m_mtx_);
	m_descriptors_.reserve(__capacity);
}


void
poll_vector::shrink_to_fit()
{
	std::lock_guard<std::mutex> lock(m_mtx_);
	m_descriptors_.shrink_to_fit();
}


void
poll_vector::clear()
{
	std::lock_guard<std::mutex> lock(m_mtx_);
	m_descriptors_.clear();
}


void
poll_vector::push_back(handle_event_pair __pair)
{
	std::lock_guard<std::mutex> lock(m_mtx_);
	struct pollfd token;
	token.fd      = __pair.first.get();
	token.events  = (short)__pair.second;
	token.revents = 0;
	m_descriptors_.push_back(token);
}


void
poll_vector::erase(unsigned int __position)
{
	std::lock_guard<std::mutex> lock(m_mtx_);
	m_descriptors_.erase(m_descriptors_.begin() + __position);
}


void
poll_vector::pop_back()
{
	std::lock_guard<std::mutex> lock(m_mtx_);
	m_descriptors_.pop_back();
}


poll_flags
poll_vector::at(unsigned int __idx)
{
	std::lock_guard<std::mutex> lock(m_mtx_);
	return static_cast<poll_flags>(m_descriptors_.at(__idx).revents);
}


poll_flags
poll_vector::operator[] (unsigned int __idx)
{
	std::lock_guard<std::mutex> lock(m_mtx_);
	return static_cast<poll_flags>(m_descriptors_[__idx].revents);
}


unsigned int
poll_vector::find(const basic_socket& __target)
{
	std::lock_guard<std::mutex> lock(m_mtx_);
	if(m_descriptors_.size() == 0)
		return static_cast<unsigned int>(-1);

	unsigned int index = 0;
	for(const auto& descriptor : m_descriptors_) {
		if(descriptor.fd == __target.get())
			return index;
		index++;
	}

	return static_cast<unsigned int>(-1); // EOF
}
