/**
 * Created by TekuConcept on July 11, 2018
 */

#include "sockets/basic_socket.h"
#include "sockets/async_pipeline.h"
#include "basic_socket_common.inc"

using namespace impact;
using namespace internal;

std::future<int>
basic_socket::send_async(
    const void*   __buffer,
    int           __length,
	message_flags __flags)
{
    ASSERT_MOVED
    try {
        async_pipeline& pipeline = async_pipeline::instance();
        return pipeline.send(this, __buffer, __length, __flags);
    }
    catch (...) { throw; }
}


std::future<int>
basic_socket::sendto_async(
    const void*    __buffer,
    int            __length,
	unsigned short __port,
	std::string    __address,
	message_flags  __flags)
{
    ASSERT_MOVED
    try {
        async_pipeline& pipeline = async_pipeline::instance();
        return pipeline.sendto(
            this,
            __buffer,
            __length,
            __port,
            __address,
            __flags
        );
    }
    catch (...) { throw; }
}


std::future<int>
basic_socket::recv_async(
    void*         __buffer,
    int           __length,
	message_flags __flags)
{
    ASSERT_MOVED
    try {
        async_pipeline& pipeline = async_pipeline::instance();
        return pipeline.recv(this, __buffer, __length, __flags);
    }
    catch (...) { throw; }
}


std::future<int>
basic_socket::recvfrom_async(
    void*                           __buffer,
    int                             __length,
    std::shared_ptr<unsigned short> __port,
	std::shared_ptr<std::string>    __address,
	message_flags                   __flags)
{
    ASSERT_MOVED
    try {
        async_pipeline& pipeline = async_pipeline::instance();
        return pipeline.recvfrom(
            this,
            __buffer,
            __length,
            __port,
            __address,
            __flags
        );
    }
    catch (...) { throw; }
}


std::future<int>
basic_socket::accept_async(basic_socket* __client)
{
    ASSERT_MOVED
    try {
        async_pipeline& pipeline = async_pipeline::instance();
        return pipeline.accept(this, __client);
    }
    catch (...) { throw; }
}
