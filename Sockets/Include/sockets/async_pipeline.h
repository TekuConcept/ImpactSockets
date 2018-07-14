/**
 * Created by TekuConcept on July 10, 2018
 */

#ifndef _IMPACT_ASYNC_PIPELINE_H_
#define _IMPACT_ASYNC_PIPELINE_H_

#include "sockets/environment.h"

#if !defined(__WINDOWS__)
	#include <sys/poll.h> // For struct pollfd, poll()
#endif
#include <cstdint>
#include <future>

#include "sockets/types.h"

namespace impact {
namespace internal {
	struct async_index {
		async_index();
	private:
		size_t value;
		friend class async_pipeline;
	};

  class async_pipeline {
  public:
    async_pipeline(const async_pipeline&) = delete;
    async_pipeline& operator=(const async_pipeline&) = delete;
    static async_pipeline& instance();

    ~async_pipeline();

    // std::future<buffer_data>& send(int socket, struct async_index* index,
		// 	const void* buffer, int length, message_flags flags);

  private:
		enum {
			SEND = 0
		};

    struct token_info {
      std::promise<buffer_data> promise;
			bool                      active;
      char*                     buffer;
      int                       length;
			message_flags             flags;
			token_info();
    };

		struct handle_info {
			bool                cancel;  // used to cancel all actions on the descriptor
			struct async_index* index;   // keep track of existing descriptors
			struct token_info   info[1]; // async data info
			handle_info();
		};

    std::vector<pollfd>      m_handles_;
    std::vector<handle_info> m_info_;

    async_pipeline();
		// void _M_create(int descriptor, struct async_index* index);
		// std::future<buffer_data>& _M_enqueue(int descriptor,
		// 	struct async_index* index, int action, poll_flags flag);
  };
  /*
    future basic_socket.send_async()
    future basic_socket.sendto_async()
    future basic_socket.recv_async()
    future basic_socket.recvfrom_async()
    future basic_socket.accept_async()
    basic_socket::close() -> wait for promises to resolve?
    socketstream(basic_socket, run_async)
  */
}}

#endif
