/**
 * Created by TekuConcept on March 16, 2018
 */

#include "IOContext.h"
#include <chrono>

using namespace Impact;

#define IOC_ERROR   -1
#define IOC_EOF      0
#define IOC_DONE     1

#define IOC_NEXT     1
#define IOC_DEQUEUED 2

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


std::future<int> IOContext::enqueue(CommunicatingSocket& socket, char* buffer,
    int length, FunctionCallback callback) {
    if(length == 0 || buffer == NULL) {
        std::promise<int> temp;
        temp.set_value(IOC_DONE);
        return temp.get_future();
    }
    else {
        std::lock_guard<std::mutex> lock(_mtxq_);
        _queue_.push_back(Entity(&socket,buffer,length,callback));
        _polltoken_.add(socket.getHandle(),POLLIN);
        _cv_.notify_one();
        return _queue_.back().promise.get_future();
    }
}


std::promise<int> IOContext::dequeue(unsigned int index) {
    std::lock_guard<std::mutex> lock(_mtxq_);
    std::promise<int> promise = std::move(_queue_[index].promise);
    auto back = _queue_.size() - 1;
    
    if(back > 0) _queue_[index] = std::move(_queue_[back]);
    VERBOSE("> Dequeued [" << index << "]");
    _queue_.pop_back();
    _polltoken_.remove(index);
    
    return std::move(promise);
}


void IOContext::dequeue(unsigned int index, int result) {
    auto promise = dequeue(index);
    try { promise.set_value(result); }
    catch (...) { VERBOSE("IOContext Error: Dequeue [set promise value]"); }
}


void IOContext::dequeue(unsigned int index, std::exception_ptr exception) {
    auto promise = dequeue(index);
    try { promise.set_exception(exception); }
    catch (...) { VERBOSE("IOContext Error: Dequeue [set promise exception]"); }
}


void IOContext::update(unsigned int& size) {
    _polltoken_.reset();
    auto result = Socket::poll(_polltoken_, _polltimeout_);
    if(result == 0) return;
    // else if(result < 0) /* error handling */
    
    unsigned int i = 0;
    do {
        {
            std::lock_guard<std::mutex> lock(_mtxq_);
            size = _queue_.size();
        }
        auto state = updateEntity(i);
        if(state == IOC_NEXT) i++;
        else size--;
    } while(i < size);
}


int IOContext::updateEntity(unsigned int index) {
    if(_polltoken_[index] & POLLHUP) {
        dequeue(index,IOC_EOF);
        return IOC_DEQUEUED;
    }
    else if(_polltoken_[index] & POLLIN) {
        try {
            auto recvCount = _queue_[index].socket->recv(
                _queue_[index].buffer,
                _queue_[index].length
            );
            _queue_[index].buffer += recvCount;
            _queue_[index].length -= recvCount;
            return checkCondition(index,recvCount);
        }
        catch (SocketException) {
            dequeue(index,std::current_exception());
            return IOC_DEQUEUED;
        }
        catch (...) { std::cerr << "Unknown Exception" << std::endl; }
    }
    return IOC_NEXT;
}

int IOContext::checkCondition(unsigned int index, ssize_t recvCount) {
    int value = 0;
    
    if(recvCount > 0) {
        if(_queue_[index].length == 0) {
            if(_queue_[index].callback == NULL) { value = IOC_DONE; }
            else {
                _queue_[index].callback(
                    _queue_[index].buffer,
                    _queue_[index].length
                );
                if(!(_queue_[index].buffer && _queue_[index].length))
                    value = IOC_DONE;
                else return IOC_NEXT;
            }
        }
        else return IOC_NEXT;
    }
    else if(recvCount == 0) value = IOC_EOF;
    else value = IOC_ERROR;
    
    dequeue(index,value);
    return IOC_DEQUEUED;
}