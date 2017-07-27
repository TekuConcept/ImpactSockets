/**
 * Created by TekuConcept on July 25, 2017
 */

#ifndef RFC_MESSAGE_H
#define RFC_MESSAGE_H

#include <string>
#include <map>
#include <vector>
#include "RFC/Const2616.h"
#include "RFC/String.h"

namespace Impact {
namespace RFC2616 {
    typedef std::pair<HEADER,std::string> HeaderToken;
    typedef std::pair<RFC2616::string,std::string> UHeaderToken;
    
    class Message {
    public:
        Message();
        
        bool addHeader(HEADER header, std::string value);
        bool addHeader(std::string header, std::string value);
        virtual std::string toString() = 0;
        
        std::string getHeaderValue(HEADER id);
        std::string getHeaderValue(std::string name, unsigned int next = 0);
        
        unsigned int major();
        unsigned int minor();

    protected:
        std::map<HEADER, std::string> _headers_;
        std::vector<UHeaderToken> _userHeaders_;
        unsigned int _major_, _minor_;
        
        virtual bool parse(std::istream &stream);
        bool parseVersion(std::string header, unsigned int &offset);
    
    private:
        bool parseHeader(std::string header);
        unsigned int matchHeaderName(std::string fieldName);
    };
}}

#endif