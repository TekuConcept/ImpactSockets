/**
 * Created by TekuConcept on July 28, 2017
 */

#ifndef RFC_WEBSOCKET_H
#define RFC_WEBSOCKET_H

#include <string>

namespace Impact {
    class IWebsocket {
    public:
        virtual bool initiateHandshake() = 0;
        
    protected:
        const std::string SECRET = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    };
}

#endif