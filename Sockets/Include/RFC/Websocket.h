/**
 * Created by TekuConcept on July 27, 2017
 */

#ifndef RFC_WEBSOCKET_H
#define RFC_WEBSOCKET_H

#include <iostream>
#include <string>
#include <random>
#include "RFC/Const6455.h"
#include "RFC/WSURI.h"
#include "RFC/RequestMessage.h"

namespace Impact {
namespace RFC6455 {
    class Websocket {
    public:
        Websocket(std::iostream &stream);
        
        void initiateClientHandshake(WSURI uri);
        bool initiateServerHandshake();
        bool acceptResponse();
    
    private:
        std::iostream& _stream_;
        std::mt19937 _engine_;
        std::uniform_int_distribution<uint8_t> _distribution_;
        std::string _key_;
        
        std::string generateRequest(WSURI uri);
        std::string generateKey();
        bool validateRequest(RFC2616::RequestMessage message, std::string &key);
        
        const std::string SECRET;
    };
}}

#endif