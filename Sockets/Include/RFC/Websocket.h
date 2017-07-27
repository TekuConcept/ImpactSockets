/**
 * Created by TekuConcept on July 27, 2017
 */

#ifndef RFC_WEBSOCKET_H
#define RFC_WEBSOCKET_H

#include <iostream>
#include <string>
#include "RFC/Const6455.h"
#include "RFC/WSURI.h"

namespace Impact {
namespace RFC6455 {
    class Websocket {
    public:
        Websocket(std::iostream &stream);
        
        void initiateHandshake(WSURI uri);
    
    private:
        std::iostream& _stream_;
        
        std::string generateRequest(WSURI uri);
    };
}}

#endif