/**
 * Created by TekuConcept on July 10, 2018
 */

#ifndef _IMPACT_ASYNC_PIPELINE_H_
#define _IMPACT_ASYNC_PIPELINE_H_

#include "sockets/environment.h" /* this include must be first */

#if !defined(__WINDOWS__)
	#include <sys/poll.h> // For struct pollfd, poll()
#endif
#include <future>
#include <vector>
#include <map>
#include <string>
#include <functional>

#include "sockets/types.h"
#include "sockets/basic_socket.h"

namespace impact {
namespace internal {
	class async_pipeline {
	public:
		async_pipeline(const async_pipeline&) = delete;
		async_pipeline& operator=(const async_pipeline&) = delete;
		static async_pipeline& instance();

		~async_pipeline();

		std::future<int> send(basic_socket* socket, const void* buffer,
			int length, message_flags flags = message_flags::NONE)
			/* throw(impact_error) */;
		std::future<int> sendto(basic_socket* socket, const void* buffer,
			int length, unsigned short port, const std::string& address,
			message_flags flags = message_flags::NONE)
			/* throw(impact_error) */;
		std::future<int> recv(basic_socket* socket, void* buffer, int length,
			message_flags flags = message_flags::NONE)
			/* throw(impact_error) */;
		std::future<int> recvfrom(basic_socket* socket, void* buffer,
			int length, unsigned short* port, std::string* address,
		 	message_flags flags = message_flags::NONE)
			/* throw(impact_error) */;
		std::future<int> accept(basic_socket* socket, basic_socket* client)
			/* throw(impact_error) */;

	private:
		enum ioaction {
			SEND     = 0,
			SENDTO   = 1,
			RECV     = 2,
			RECVFROM = 3,
			ACCEPT   = 4
		};

		enum class action_state {
			PENDING,
			FREE
		};

		struct action_info {
			action_state         state;
			std::promise<int>    promise;
			std::function<int()> iofunction;
			action_info();
		};

		struct handle_info {
			size_t             pollfd_index;
			struct action_info input;
			struct action_info output;
			handle_info();
		};

		std::vector<pollfd>        m_handles_;
		std::map<int, handle_info> m_info_;

		async_pipeline();

		int _M_create_pollfd(int);
		std::future<int> _M_enqueue(int, ioaction, const std::function<int()>&)
			/* throw(impact_error) */;
	};
	/* socketstream(basic_socket, run_async) */
}}

#endif
