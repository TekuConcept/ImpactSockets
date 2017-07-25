/**
 * Created by TekuConcept on July 24, 2017
 */

#ifndef RFC_REQUEST_MESSAGE_H
#define RFC_REQUEST_MESSAGE_H

#include <string>
#include <vector>
#include "RFC/RFC2616Const.h"

namespace Impact {
namespace RFC2616 {
    typedef std::pair<HEADER,std::string> StringHeaderPair;
    typedef std::pair<std::string, std::string > StringStringPair;

    class RequestMessage {
    public:
        RequestMessage(Request::METHOD method, std::string uri);
        void addHeader(HEADER header, std::string value);
        void addUserHeader(std::string header, std::string value);
        std::string toString();
    private:
        Request::METHOD _method_;
        std::string _uri_;
        std::vector<StringHeaderPair> _headers_;
        std::vector<StringStringPair> _userHeaders_;
    };
}}

#endif