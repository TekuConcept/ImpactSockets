/**
 * Created by TekuConcept on July 25, 2018
 */

#ifndef _IMPACT_WORKER_THREAD_H_
#define _IMPACT_WORKER_THREAD_H_

#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace impact {
namespace internal {
    class worker_thread {
    public:
        worker_thread();
        virtual ~worker_thread();
    
    protected:
        std::condition_variable m_thread_cv_;
        
        virtual bool _M_has_work();
        virtual void _M_dowork();

    private:
        std::thread             m_thread_;
        std::mutex              m_thread_mtx_;
        std::atomic<bool>       m_ready_;
        std::atomic<bool>       m_shutting_down_;
        
        void _M_begin();
        void _M_end();
    };
}}

#endif