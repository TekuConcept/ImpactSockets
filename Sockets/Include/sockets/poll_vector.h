/**
 * Created by TekuConcept on June 19, 2018
 */

#ifndef _IMPACT_POLL_VECTOR_H_
#define _IMPACT_POLL_VECTOR_H_

#include <vector>
#include <mutex>
#include <initializer_list>

#include "sockets/environment.h"
#include "sockets/basic_socket.h"
#include "sockets/types.h"

#if !defined(__WINDOWS__)
	#include <sys/poll.h> // For struct pollfd, poll()
#endif

namespace impact {
	typedef std::pair<const basic_socket&, poll_flags> handle_event_pair;
	typedef std::initializer_list<handle_event_pair> poll_initializer;

	class poll_vector {
	public:
		poll_vector();
		poll_vector(poll_initializer handles);
		virtual ~poll_vector();

		void reset_events();

		// -- capacity --
		bool empty() const;
		unsigned int size() const;
		void reserve(unsigned int capacity);
		void shrink_to_fit();


		// -- modifiers --
		void clear();
		void erase(unsigned int position);
		void push_back(handle_event_pair pair);
		void pop_back();

		// -- accessors --
		// access return events at index
		poll_flags at(unsigned int idx);
		poll_flags operator[] (unsigned int idx);
		unsigned int find(const basic_socket& target);

		friend class probe;

	protected:
		std::vector<struct pollfd> m_descriptors_;
		std::mutex m_mtx_;
	};
}

#endif
