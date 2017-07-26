/**
 * Created by TekuConcept on July 25, 2017
 */

#ifndef RFC_RESPONSE_MESSAGE_H
#define RFC_RESPONSE_MESSAGE_H

#include "RFC/Message.h"

namespace Impact {
namespace RFC2616 {
    class ResponseMessage : public Message {
    public:
        ResponseMessage();
        
        std::string toString();
    };
}}

#endif