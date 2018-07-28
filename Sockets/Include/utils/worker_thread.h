/**
 * Created by TekuConcept on July 25, 2018
 */

#ifndef _IMPACT_WORKER_THREAD_H_
#define _IMPACT_WORKER_THREAD_H_

#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include <condition_variable>

namespace impact {
namespace internal {
    class worker_thread {
    public:
        worker_thread();
        virtual ~worker_thread();
        
        struct object { /* extend from me */ };
        typedef std::shared_ptr<object> object_ptr;
    
    protected:
        virtual bool _M_has_work();
        virtual void _M_dowork();
        void _M_register_obj(object_ptr obj);

    private:
        std::thread             m_thread_;
        std::condition_variable m_thread_cv_;
        std::mutex              m_thread_mtx_;
        std::atomic<bool>       m_ready_;
        std::atomic<bool>       m_shutting_down_;
        std::vector<object_ptr> m_objects_;
        
        void _M_begin();
        void _M_end();
    
    protected: /* inline */
        inline void _M_notify_one() {
            m_thread_cv_.notify_one();
        }
    };
}}

#endif