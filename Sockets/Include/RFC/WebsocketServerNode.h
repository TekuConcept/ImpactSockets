/**
 * Created by TekuConcept on July 28, 2017
 */

#ifndef RFC_WEBSOCKET_SERVER_NODE_H
#define RFC_WEBSOCKET_SERVER_NODE_H

#include "RFC/Websocket.h"
#include "RFC/RequestMessage.h"
#include <sstream>

namespace Impact {
namespace RFC6455 {
    class WebsocketServerNode : public Websocket {
    public:
        WebsocketServerNode(std::iostream &stream);
        bool initiateHandshake();
        
    private:
        bool validateRequest(RFC2616::RequestMessage message);
    };
}}

#endif