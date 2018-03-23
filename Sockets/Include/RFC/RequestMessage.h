/**
 * Created by TekuConcept on July 24, 2017
 */

#ifndef RFC_REQUEST_MESSAGE_H
#define RFC_REQUEST_MESSAGE_H

#include <string>
#include <vector>
#include <istream>
#include "RFC/Const2616.h"
#include "RFC/Message.h"

namespace Impact {
namespace RFC2616 {
    class RequestMessage : public Message {
    public:
        RequestMessage(METHOD method, std::string uri);
        RequestMessage(std::string message);

        std::string toString();
        
        METHOD method();
        std::string resource();
        
        static RequestMessage tryParse(std::istream &request, bool &success);
        static bool validate(std::string request);

    private:
        METHOD _method_;
        std::string _uri_;
        
        RequestMessage();
        bool parse(std::istream &request);
        bool parseRequestHeader(std::string header);
        bool parseRequestMethod(std::string header);
        bool parseRequestURI(std::string header, unsigned int &offset);
    };
}}

#endif