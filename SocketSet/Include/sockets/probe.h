/**
 * Created by TekuConcept on July 8, 2018
 */

#ifndef _IMPACT_PROBE_H_
#define _IMPACT_PROBE_H_

#include <vector>

#include "sockets/basic_socket.h"
#include "sockets/types.h"

namespace impact {
    typedef struct poll_handle {
        int   socket;
        short events;
        short return_events;
        poll_handle();
    } PollHandle;
    
    
    int select(
        std::vector<basic_socket*> readHandles,
        std::vector<basic_socket*> writeHandles,
        int timeout=-1, unsigned int microTimeout=0);


    int poll(std::vector<poll_handle>* handles, int timeout=-1);
}

#endif
