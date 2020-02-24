/**
 * Created by TekuConcept on July 10, 2018
 */

#ifndef IMPACT_ASYNC_PIPELINE_H
#define IMPACT_ASYNC_PIPELINE_H

#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <vector>
#include <map>

#include <memory>
#include <functional>

#include "sockets/probe.h"
#include "sockets/basic_socket.h"

#if defined(__OS_WINDOWS__)
    /* used as async_option, windows defines it as 0 */
    #undef IGNORE
#endif

namespace impact {
namespace internal {
    class async_object;
    typedef std::shared_ptr<async_object> async_object_ptr;
    
    
    class async_pipeline {
    public:
        async_pipeline(const async_pipeline&) = delete;
        async_pipeline& operator=(const async_pipeline&) = delete;
        static async_pipeline& instance();

        ~async_pipeline();
        
        void granularity(int milliseconds);
        void add_object(int socket, async_object_ptr object) /* throw(impact_error) */;
        void remove_object(int socket) /* throw(impact_error) */;
        void notify();

    private:
        async_pipeline();
        typedef std::pair<int, async_object_ptr> handle_info;
        
        std::thread                    m_thread_;
        std::condition_variable        m_thread_cv_;
        std::mutex                     m_thread_mtx_;
        std::atomic<bool>              m_thread_ready_;
        std::atomic<bool>              m_thread_closing_;
        std::atomic<bool>              m_thread_has_work_;
        std::atomic<int>               m_thread_pending_;
        
        std::mutex                     m_work_mtx_;
        std::vector<poll_handle>       m_work_handles_;
        std::map<int,async_object_ptr> m_work_info_;
        std::vector<handle_info>       m_work_pending_;
        std::vector<int>               m_work_removed_;
        std::atomic<int>               m_poll_granularity_;
        
        const int                      k_default_granularity_ = 50;

        void _M_notify_pending(int);
        void _M_copy_pending_to_queue();
        void _M_remove_pending_from_queue();
        bool _M_update_handles();
        void _M_dowork();
        void _M_begin();
        void _M_end();
    };
    
    
    typedef enum class async_option {
        CONTINUE,
        IGNORE,
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
