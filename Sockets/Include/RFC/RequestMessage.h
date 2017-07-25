/**
 * Created by TekuConcept on July 24, 2017
 */

#ifndef RFC_REQUEST_MESSAGE_H
#define RFC_REQUEST_MESSAGE_H

#include <string>
#include <vector>
#include <istream>
#include "RFC/RFC2616Const.h"

namespace Impact {
namespace RFC2616 {
    typedef std::pair<HEADER,std::string> StringHeaderPair;
    typedef std::pair<std::string, std::string > StringStringPair;

    class RequestMessage {
    public:
        RequestMessage(Request::METHOD method, std::string uri);
        RequestMessage(std::string message);
        
        void addHeader(HEADER header, std::string value);
        void addUserHeader(std::string header, std::string value);
        std::string toString();
        
        Request::METHOD method();
        std::string resource();
        unsigned int major();
        unsigned int minor();
        
        static RequestMessage tryParse(std::istream &request, bool &success);
        static bool validate(std::string request);

    private:
        Request::METHOD _method_;
        std::string _uri_;
        std::vector<StringHeaderPair> _headers_;
        std::vector<StringStringPair> _userHeaders_;
        unsigned int _major_, _minor_;
        
        RequestMessage();
        bool parse(std::istream &request);
        bool parseRequestHeader(std::string header);
        bool parseRequestMethod(std::string header);
        bool parseRequestURI(std::string header, unsigned int &offset);
        bool parseRequestVersion(std::string header, unsigned int &offset);
    };
}}

// typedef struct Info {
//                 METHOD method;
//                 std::string requestURI;
//                 std::vector<std::string> headers;
//                 std::string body;
//             } Info;

#endif