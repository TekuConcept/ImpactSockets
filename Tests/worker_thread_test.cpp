/**
 * Created by TekuConcept on July 25, 2018
 */

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

#include "utils/worker_thread.h"

#define VERBOSE(x) std::cout << x << std::endl


template <class T>
class printer : impact::internal::worker_thread {
public:
    printer();
    ~printer();
    void dowork(T value);

protected:
    virtual bool _M_has_work();
    virtual void _M_dowork();
    
private:
    std::mutex     m_var_mtx_;
    std::vector<T> m_queue_;
    std::vector<T> m_pending_;
    
    void _M_copy_pending_to_queue();
};


int main() {
    VERBOSE("- BEGIN -");
    
    {
        printer<int> object;
        object.dowork(2);
        
        for (int i = 0; i < 5; i++)
            object.dowork(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
        for (int j = 5; j >= 0; j--)
            object.dowork(j);
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    }
    
    
    VERBOSE("- END OF LINE -");
    return 0;
}


template <class T>
printer<T>::printer()
{}


template <class T>
printer<T>::~printer()
{}


template <class T>
void
printer<T>::_M_copy_pending_to_queue()
{
    std::lock_guard<std::mutex> lock(m_var_mtx_);
    if (m_pending_.size() > 0) {
        if (m_queue_.size() == 0) 
            m_queue_ = m_pending_;
        else {
            auto destination = m_queue_.size();
            m_queue_.resize(m_queue_.size() + m_pending_.size());
            std::copy(
                m_pending_.begin(),
                m_pending_.end(),
                m_queue_.begin() + destination
            );
        }
        m_pending_.clear();
    }
}


template <class T>
bool
printer<T>::_M_has_work()
{
    std::lock_guard<std::mutex> lock(m_var_mtx_);
    return m_pending_.size() != 0 || m_queue_.size() != 0;
}


template <class T>
void
printer<T>::_M_dowork()
{
    while (_M_has_work()) {        
        _M_copy_pending_to_queue();
        std::cout << "> Data: ";
        for (const auto& token : m_queue_)
            std::cout << " " << token;
        std::cout << std::endl;
        m_queue_.pop_back();
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}


template <class T>
void
printer<T>::dowork(T __data)
{
    std::lock_guard<std::mutex> lock(m_var_mtx_);
    m_pending_.push_back(__data);
    m_thread_cv_.notify_one();
}
