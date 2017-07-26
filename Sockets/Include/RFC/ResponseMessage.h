/**
 * Created by TekuConcept on July 25, 2017
 */

#ifndef RFC_RESPONSE_MESSAGE_H
#define RFC_RESPONSE_MESSAGE_H

#include "RFC/Message.h"
#include "RFC/Const2616.h"
#include <string>

namespace Impact {
namespace RFC2616 {
    class ResponseMessage : public Message {
    public:
        ResponseMessage(STATUS code);
        
        std::string toString();
        STATUS status();
        
        static ResponseMessage tryParse(std::istream &request, bool &success);
        static bool validate(std::string request);
    
    private:
        STATUS _status_;
        
        ResponseMessage();
        bool parse(std::istream &response);
        bool parseResponseHeader(std::string header);
        bool parseStatusCode(std::string header, unsigned int &offset);
    };
}}

#endif