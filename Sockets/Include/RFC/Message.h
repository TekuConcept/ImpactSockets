/**
 * Created by TekuConcept on July 25, 2017
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
        Message();
        
        void addHeader(HEADER header, std::string value);
        void addUserHeader(std::string header, std::string value);
        virtual std::string toString() = 0;
        
        unsigned int major();
        unsigned int minor();

    protected:
        std::vector<StringHeaderPair> _headers_;
        std::vector<StringStringPair> _userHeaders_;
        unsigned int _major_, _minor_;
        
        virtual bool parse(std::istream &stream);
        bool parseVersion(std::string header, unsigned int &offset);
    
    private:
        bool parseHeader(std::string header);
    };
}}

#endif