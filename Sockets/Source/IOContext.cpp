/**
 * Created by TekuConcept on March 16, 2018
 */

#include "IOContext.h"
#include <chrono>

using namespace Impact;

#define IOC_ERROR -1
#define IOC_EOF    0
#define IOC_DONE   1

#include <iostream>
#define VERBOSE(x) std::cout << x << std::endl

IOContext::Entity::Entity(CommunicatingSocket* s, char* b, int l,
    FunctionCallback f) : socket(s), buffer(b), length(l), callback(f) {}

IOContext::IOContext() : _futureReady_(_promiseReady_.get_future()),
    _active_(true), _polltimeout_(1000) {
    _service_ = std::thread([&](){
        std::unique_lock<std::mutex> lock(_mtx_);
        _promiseReady_.set_value();
        while(_active_) {
            if(_cv_.wait_for(lock,std::chrono::seconds(1)) ==
                std::cv_status::timeout) {
                std::lock_guard<std::mutex> lock(_mtxq_);
                if(_queue_.size() == 0) continue;
            }
            unsigned int size = 0;
            do { update(size); } while(_active_ && size > 0);
        }
    });
}

IOContext::~IOContext() {
    _active_ = false;
    _cv_.notify_one();
    _service_.join();
}

void IOContext::update(unsigned int& size) {
    std::lock_guard<std::mutex> lock(_mtxq_);
    size = _queue_.size();
    
    _polltoken_.reset();
    auto result = Socket::poll(_polltoken_, _polltimeout_);
    if(result == 0) return;
    // else if(result < 0) /* error handling */
    for(unsigned int i = 0; i < size; i++) {
        updateEntity(i, size);
    }
}

void IOContext::updateEntity(unsigned int& i, unsigned int& s) {
    if(_polltoken_[i] & POLLHUP) { dequeue(i,s,IOC_EOF); }
    else if(_polltoken_[i] & POLLIN) {
        try {
            auto rlength = _queue_[i].socket->recv(
                _queue_[i].buffer,
                _queue_[i].length
            );
            _queue_[i].buffer += rlength;
            _queue_[i].length -= rlength;
            updateState(i,s,rlength);
        } catch (...) { dequeue(i,s,0,true); }
    }
}

bool IOContext::updateState(unsigned int& i, unsigned int& s,
    ssize_t rlength) {
    int value = 0;
    if(rlength > 0) {
        if(_queue_[i].length == 0) {
            if(_queue_[i].callback == NULL) { value = IOC_DONE; }
            else {
                _queue_[i].callback(_queue_[i].buffer, _queue_[i].length);
                if(_queue_[i].buffer == NULL || _queue_[i].length == 0)
                    value = IOC_DONE;
                else return false;
            }
        }
        else return false;
    }
    else if(rlength == 0) value = IOC_EOF; 
    else value = IOC_ERROR;
    dequeue(i,s,value);
    return true;
}

std::future<int> IOContext::enqueue(CommunicatingSocket& socket, char* buffer,
    int length, FunctionCallback callback) {
    if(length == 0 || buffer == NULL) {
        std::promise<int> temp;
        temp.set_value(-1);
        return temp.get_future();
    }
    std::lock_guard<std::mutex> lock(_mtxq_);
    _queue_.push_back(Entity(&socket,buffer,length,callback));
    _polltoken_.add(socket.getHandle(),POLLIN);
    _cv_.notify_one();
    return _queue_.back().promise.get_future();
}

void IOContext::dequeue(unsigned int& index, unsigned int& size, int pvalue,
    bool exception) {
    try {
        if(exception)
             _queue_[index].promise.set_exception(std::current_exception());
        else _queue_[index].promise.set_value(pvalue);
    } catch (...) {
        std::cout << "Error In IOContext Dequeue Process" << std::endl;
    }
    auto back = _queue_.size() - 1;
    if(back > 0) {
        _queue_[index] = std::move(_queue_[back]);
        // _queue_[index].socket   = _queue_[back].socket;
        // _queue_[index].buffer   = entity.buffer;
        // _queue_[index].length   = entity.length;
        // _queue_[index].promise  = std::move(entity.promise);
        // _queue_[index].callback = std::move(entity.callback);
    }
    VERBOSE("> Dequeued [" << index << "]");
    // (void)index;
    // (void)pvalue;
    // (void)exception;
    _queue_.pop_back();
    _polltoken_.remove(index);
    size--;
    index--;
}
