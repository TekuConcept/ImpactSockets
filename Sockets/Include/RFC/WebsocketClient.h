/**
 * Created by TekuConcept on July 27, 2017
 */

#ifndef RFC_WEBSOCKET_CLIENT_H
#define RFC_WEBSOCKET_CLIENT_H

#include "RFC/IWebsocket.h"
#include "RFC/WSURI.h"
#include <iostream>
#include <random>

namespace Impact {
namespace RFC6455 {
    class WebsocketClient : public IWebsocket {
    public:
        WebsocketClient(std::iostream &stream, WSURI uri);
        
        bool initiateHandshake();
        bool acceptResponse();
    
    private:
        std::iostream& _stream_;
        std::string _key_;
        WSURI _uri_;
        std::mt19937 _engine_;
        std::uniform_int_distribution<uint8_t> _distribution_;

        std::string generateKey();
    };
}}

#endif