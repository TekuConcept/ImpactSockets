/**
 * Created by TekuConcept on July 11, 2018
 */

#include "sockets/async_pipeline.h"

#include <cstdint>
#include <stdexcept>

#include "sockets/environment.h"
#include "sockets/async_error.h"

using namespace impact;
using namespace internal;

// async_index::async_index()
// : value((size_t)(-1))
// {}


// async_pipeline::token_info::token_info()
// : active(false), buffer(NULL), length(0), flags(message_flags::NONE)
// {}


// async_pipeline::handle_info::handle_info()
// : cancel(false), index(NULL)
// {}


async_pipeline&
async_pipeline::instance()
{
  static async_pipeline unit;
  return unit;
}


async_pipeline::async_pipeline()
{
  // lazy startup thread
}


async_pipeline::~async_pipeline()
{
  // shutdown thread
}


// void
// _M_create(
//   int                 __descriptor,
//   struct async_index* __index)
// {
//   struct pollfd token;
//   token.fd      = __descriptor;
// 	token.events  = 0;
// 	token.revents = 0;
//   m_handles_.push_back(token);
//
//   __index->value = m_handles_.size() - 1;
//   struct handle_info info;
//   info.index = __index;
//   m_info_.push_back(std::move(info));
// }


// std::future<buffer_data>&
// _M_enqueue(
//   int                 __descriptor,
//   struct async_index* __index,
//   int                 __action,
//   poll_flags          __flag)
// {
//   // assumptions:
//   // __socket is valid
//   // __index is not NULL
//   // __action is a valid action id
//   // __index.value is within range
//
//   struct token_info* info;
//   if (__index->value == (size_t)(-1))
//     _M_create(__descriptor, __index);
//
//   info = &m_info_.at(__index->value).info[__action];
//   if (info->active)
//     throw async_error(
//       "async_pipeline::_M_enqueue()\n"
//       "Called before promise resolved"
//     );
//
//   m_handles_.at(__index->value).events |= (short)flag;
//   info->promise = std::promise<buffer_data>();
//   info->active  = true;
//   info->buffer  = __buffer;
//   info->length  = __length;
//   info->flags   = __flags;
//
//   return info->promise.get_future();
// }


// std::future<buffer_data>&
// async_pipeline::send(
//   int descriptor      __socket,
//   struct async_index* __index,
//   const void*         __buffer,
//   int                 __length,
//   message_flags       __flags);
// {
  /*
  if (handle exists):
    get handle
    if (handle is active):
      throw async_error
  else:
    create handle
  set handle event as POLL::OUT
  set handle as active
  return future
  */

  /*
  struct token_info {
    std::promise<buffer_data> promise;
    bool                      active;
    char*                     buffer;
    int                       length;
    token_info();
  };

  struct handle_info {
    bool                cancel;  // used to cancel all actions on the descriptor
    struct async_index* index;   // keep track of existing descriptors
    struct token_info   info[1]; // async data info
    handle_info();
  };
  */



//   return token->promise.get_future();
// }
