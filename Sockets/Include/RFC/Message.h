/**
 * 
 */

#ifndef RFC_MESSAGE_H
#define RFC_MESSAGE_H

#include <string>
#include <vector>
#include "RFC/Const2616.h"

namespace Impact {
namespace RFC2616 {
    typedef std::pair<HEADER,std::string> StringHeaderPair;
    typedef std::pair<std::string, std::string > StringStringPair;
    
    class Message {
    public:
        virtual bool parse(std::istream &request);
        
        void addHeader(HEADER header, std::string value);
        void addUserHeader(std::string header, std::string value);
        virtual std::string toString() = 0;

    protected:
        std::vector<StringHeaderPair> _headers_;
        std::vector<StringStringPair> _userHeaders_;
    };
}}

#endif