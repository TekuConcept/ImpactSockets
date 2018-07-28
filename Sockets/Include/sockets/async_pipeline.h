/**
 * Created by TekuConcept on July 10, 2018
 */

#ifndef _IMPACT_ASYNC_PIPELINE_H_
#define _IMPACT_ASYNC_PIPELINE_H_

#include <memory>
#include <atomic>
#include <mutex>
#include <functional>
#include <future>
#include <vector>
#include <map>

#include "utils/worker_thread.h"
#include "sockets/probe.h"
#include "sockets/basic_socket.h"

namespace impact {
namespace internal {
	class async_object;
	typedef std::shared_ptr<async_object> async_object_ptr;
	template <class T>
	struct type_object : worker_thread::object { T type; };
	template <class T>
	using type_object_ptr = std::shared_ptr<type_object<T>>;
	
	
	class async_pipeline : worker_thread {
	public:
		async_pipeline(const async_pipeline&) = delete;
		async_pipeline& operator=(const async_pipeline&) = delete;
		static async_pipeline& instance();

		~async_pipeline();
		void granularity(int milliseconds);
		void add_object(int socket, async_object_ptr object)
			/* throw(impact_error) */;
		void remove_object(int socket)
			/* throw(impact_error) */;

	protected:
		virtual bool _M_has_work();
		virtual void _M_dowork();

	private:
		typedef std::pair<int, async_object_ptr> handle_info;
		
		type_object_ptr<std::mutex>                     m_var_mtx_;
		type_object_ptr<std::vector<poll_handle>>       m_handles_;
		type_object_ptr<std::map<int,async_object_ptr>> m_info_;
		type_object_ptr<std::vector<handle_info>>       m_pending_add_;
		type_object_ptr<std::vector<int>>               m_pending_remove_;
		type_object_ptr<std::atomic<bool>>              m_shutting_down_;
		type_object_ptr<std::atomic<int>>               m_granularity_;

		std::atomic<int>                                m_has_work_;

		async_pipeline();
		void _M_copy_pending_to_queue(std::vector<poll_handle>*,
			std::map<int, async_object_ptr>*);
		void _M_remove_pending_from_queue(std::vector<poll_handle>*,
			std::map<int, async_object_ptr>*);
	};
	
	
	typedef enum class async_option {
		CONTINUE,
		TOGGLE,
		QUIT
	} AsyncOption;
	
	
	class async_object {
	public:
		virtual async_option async_callback(poll_handle* handle,
			socket_error error) = 0;
	};
	
	
	class async_functor : public async_object {
	public:
		async_functor(std::function<async_option(poll_handle*,socket_error)> callback);
		virtual ~async_functor();
		virtual async_option async_callback(poll_handle*,socket_error);
	protected:
		std::function<async_option(poll_handle*,socket_error)> m_callback_;
	};
	/* socketstream(basic_socket, run_async) */
}}

#endif
