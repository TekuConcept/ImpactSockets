/**
 * Created by TekuConcept on July 24, 2017
 */

#ifndef REQUEST_MESSAGE_H
#define REQUEST_MESSAGE_H

#include <string>
#include <vector>
#include "RFC/RFC2616Const.h"

namespace Impact {
namespace RFC2616 {
namespace Request {

    class Message {
    public:
        Message(METHOD method, std::string uri);
        void addHeader(HEADER header, std::string value);
        std::string toString();
    private:
        METHOD _method_;
        std::string _uri_;
    };

}}}

#endif