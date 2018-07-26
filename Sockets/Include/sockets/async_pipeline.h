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
#include "utils/worker_thread.h"

namespace impact {
namespace internal {
	class async_pipeline : worker_thread {
	public:
		async_pipeline(const async_pipeline&) = delete;
		async_pipeline& operator=(const async_pipeline&) = delete;
		static async_pipeline& instance();

		~async_pipeline();
		void granularity(int milliseconds);

	protected:
		virtual bool _M_has_work();
        virtual void _M_dowork();

	private:
		std::mutex                  m_var_mtx_;
		std::vector<pollfd>         m_handles_;
		// std::map<int, handle_info>  m_info_;
		std::vector<int>            m_pending_;
		std::atomic<int>            m_granularity_;

		async_pipeline();
		
		// void _M_enqueue(pending_handle*);
		void _M_copy_pending_to_queue();
		int _M_create_pollfd(int);
		void _M_recover_fetal(const std::string&);
		void _M_process_events();
	};
	/* socketstream(basic_socket, run_async) */
}}

#endif
