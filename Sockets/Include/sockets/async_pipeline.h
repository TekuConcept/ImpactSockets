/**
 * Created by TekuConcept on July 10, 2018
 */

#ifndef _IMPACT_ASYNC_PIPELINE_H_
#define _IMPACT_ASYNC_PIPELINE_H_

#include <memory>
#include <functional>
#include <vector>
#include <map>

#include "utils/worker_thread.h"
#include "sockets/probe.h"
#include "sockets/basic_socket.h"

namespace impact {
namespace internal {
	class async_object {
		friend class async_pipeline;
	protected:
		virtual void async_callback(poll_handle* handle,
			socket_error error) = 0;
	};
	typedef std::shared_ptr<async_object> async_object_ptr;
	
	class async_functor : public async_object {
	public:
		async_functor(std::function<void(poll_handle*,socket_error)> callback);
		virtual ~async_functor();
	protected:
		std::function<void(poll_handle*,socket_error)> m_callback_;
		virtual void async_callback(poll_handle*,socket_error);
	};
	
	class async_pipeline : worker_thread {
	public:
		async_pipeline(const async_pipeline&) = delete;
		async_pipeline& operator=(const async_pipeline&) = delete;
		static async_pipeline& instance();

		~async_pipeline();
		void granularity(int milliseconds);
		void add_object(const basic_socket* socket, async_object_ptr object)
			/* throw(impact_error) */;
		void remove_object(const basic_socket* socket)
			/* throw(impact_error) */;

	protected:
		virtual bool _M_has_work();
        virtual void _M_dowork();

	private:
		typedef std::pair<int, async_object_ptr> handle_info;
		
		std::mutex                      m_var_mtx_;
		std::vector<poll_handle>        m_handles_;
		std::map<int, async_object_ptr> m_info_;
		std::vector<handle_info>        m_pending_add_;
		std::vector<int>                m_pending_remove_;
		std::atomic<int>                m_granularity_;

		async_pipeline();
		void _M_copy_pending_to_queue();
		void _M_remove_pending_from_queue();
	};
	/* socketstream(basic_socket, run_async) */
}}

#endif
