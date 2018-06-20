/**
 * Created by TekuConcept on June 19, 2018
 */

#ifndef _SOCKET_POLL_TABLE_H_
#define _SOCKET_POLL_TABLE_H_

#include "SocketHandle.h"
#include "SocketTypes.h"
#include <vector>
#include <mutex>
#include <initializer_list>

#if !defined(_MSC_VER)
	#include <sys/poll.h> // For struct pollfd, poll()
#endif

namespace Impact {
	typedef std::pair<const SocketHandle&,PollFlags> HandleEventPair;
	typedef std::initializer_list<HandleEventPair> PollInitializer;

	class SocketPollTable {
		protected:
			std::vector<struct pollfd> _descriptors_;
			std::mutex _mtx_;

		public:
			SocketPollTable();
			SocketPollTable(PollInitializer handles);
			~SocketPollTable();

			void resetEvents();
			
			// -- capacity --
			bool empty() const;
			unsigned int size() const;
			void reserve(unsigned int capacity);
			void shrink_to_fit();


			// -- modifiers --
			void clear();
			void erase(unsigned int position);
			void push_back(HandleEventPair pair);
			void pop_back();

			// -- accessors --
			// access return events at index
			const short& at(unsigned int idx);
			const short& operator[] (unsigned int idx);
			unsigned int find(const SocketHandle& target);

			friend class SocketInterface;
	};
}

#endif