/**
 * Created by TekuConcept on June 19, 2018
 */

#ifndef _SOCKET_POLL_TABLE_H_
#define _SOCKET_POLL_TABLE_H_

#include "SocketHandle.h"
#include <vector>

#if !defined(_MSC_VER)
	#include <sys/poll.h> // For struct pollfd, poll()
#endif

namespace Impact {
	class SocketPollTable {
		protected:
			std::vector<struct pollfd> _descriptors_;

		public:
			SocketPollTable();
			// SocketPollTable(initializer_list<SocketHandle&> handles);
			~SocketPollTable();

			void resetEvents();
			
			// -- capacity --
			bool empty() const;
			unsigned int size() const;
			void reserve(unsigned int capacity);
			void shrink_to_fit();


			// -- modifiers --
			void clear();
			void erase(int position);
			void push_back(const SocketHandle& handle, int events);
			void pop_back();

			// -- accessors --
			// access return events at index
			short& at(int idx);
			short& operator[] (int idx);
	};
}

#endif