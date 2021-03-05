/**
 * Created by TekuConcept on February 13, 2021
 */

#include "sockets/async_socket.h"

using namespace impact;


tcp_client_t
async_socket::create_tcp_client(event_loop_t __event_loop)
{ return __event_loop->create_tcp_client(); }


tcp_server_t
async_socket::create_tcp_server(event_loop_t __event_loop)
{ return __event_loop->create_tcp_server(); }
