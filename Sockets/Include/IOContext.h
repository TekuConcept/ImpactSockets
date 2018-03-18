/**
 * Created by TekuConcept on March 16, 2018
 * 
 * IOContext provides dedicated IO thread(s) for asynchronously reading data.
 */

#ifndef _IO_CONTEXT_H_
#define _IO_CONTEXT_H_

#include "Sockets.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>

namespace Impact {
    class IOContext {
        typedef std::function<void (char*&,int&)> FunctionCallback;
        typedef struct Entity {
            CommunicatingSocket* socket;
            char* buffer;
            int length;
            std::promise<int> promise;
            FunctionCallback callback;
            Entity(CommunicatingSocket*,char*,int,FunctionCallback);
        } Entity;
        
        std::thread _service_;
        std::mutex _mtx_, _mtxq_;
        std::condition_variable _cv_;
        
        std::promise<void> _promiseReady_;
        std::future<void> _futureReady_;
        std::atomic<bool> _active_;
        
        std::vector<Entity> _queue_;
        SocketPollToken _polltoken_;
        std::atomic<int> _polltimeout_;
        
        void update(unsigned int&);
        void updateEntity(unsigned int&);
        bool updateState(unsigned int,ssize_t);
        void dequeue(unsigned int,int,bool=false);
        
    public:
        IOContext();
        ~IOContext();
        
        std::future<int> enqueue(CommunicatingSocket& socket, char* buffer,
            int length, FunctionCallback whenDone);
    };
}

#endif