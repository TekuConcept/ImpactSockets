/**
 * Created by TekuConcept on March 16, 2018
 */

#include "IOContext.h"
#include <chrono>

using namespace Impact;

IOContext::Entity::Entity(SocketHandle& h, char* b, int l, FunctionCallback f) :
    handle(h), buffer(b), length(l), callback(f) {}

IOContext::IOContext() : _futureReady_(_promiseReady_.get_future()),
    _active_(true) {
    _service_ = std::thread([&](){
        std::unique_lock<std::mutex> lock(_mtx_);
        _promiseReady_.set_value();
        while(_active_) {
            if(_cv_.wait_for(lock,std::chrono::seconds(1)) ==
                std::cv_status::timeout) {
                std::lock_guard<std::mutex> lock(_mtxq_);
                if(_queue_.size() == 0) continue;
            }
            unsigned int size;
            do { update(size); } while(_active_ && size > 0);
        }
    });
}

IOContext::~IOContext() {
    _active_ = false;
    _cv_.notify_one();
    _service_.join();
}

void IOContext::update(unsigned int&) {
    /*
    if(poll(timeout) == timeout) return;
    foreach(entity : polltoken) {
        result = read(entity.handle, entity.buffer, entity.length);
        entity.length -= result;
        if(entity.length == 0) {
            if(entity.callback == NULL) dequeue;
            else {
                entity.callback(entity.buffer, entity.length);
                if(entity.length == 0) dequeue;
            }
        }
    }
     */
}

std::future<int> IOContext::enqueue(SocketHandle& handle, char* buffer,
    int length, FunctionCallback callback) {
    std::lock_guard<std::mutex> lock(_mtxq_);
    _queue_.push_back(Entity(handle,buffer,length,callback));
    _cv_.notify_one();
    return _queue_.back().promise.get_future();
}