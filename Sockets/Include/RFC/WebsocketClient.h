/**
 * Created by TekuConcept on July 27, 2017
 */

#ifndef RFC_WEBSOCKET_CLIENT_H
#define RFC_WEBSOCKET_CLIENT_H

#include "RFC/Websocket.h"
#include "RFC/WSURI.h"
#include <iostream>

namespace Impact {
namespace RFC6455 {
    class WebsocketClient : public Websocket {
    public:
        WebsocketClient(std::iostream &stream, WSURI uri);
        
        bool initiateHandshake();
        bool acceptResponse();
    
    private:
        WSURI _uri_;

        std::string generateKey();
    };
}}

#endif