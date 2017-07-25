/**
 * Created by TekuConcept on July 25, 2017
 */

#include "RFC/URI.h"
#include "RFC/Const2616.h"
#include <sstream>
#include <exception>

using namespace Impact;
using namespace RFC2616;

URI::URI() {
    
}

URI::URI(std::string uri) {
    if(!parse(uri)) throw std::exception();
}

std::string URI::scheme() {
    return _scheme_;
}

std::string URI::host() {
    return _host_;
}

std::string URI::resource() {
    return _resource_;
}

unsigned int URI::port() {
    return _port_;
}

bool URI::secure() {
    return _scheme_ == "https";
}

bool URI::validate(std::string uri) {
    URI _uri_;
    return _uri_.parse(uri);
}

URI URI::tryParse(std::string uri, bool &success) {
    URI _uri_;
    success = _uri_.parse(uri);
    return _uri_;
}

bool URI::parse(std::string uri) {
    if(!parseScheme(uri)) return false;
    unsigned int idx = _scheme_.length() + 3; // skip "://"
    
    if(idx >= uri.length()) return false;
    if(uri[idx] == '[') {
        // IPv6 enclosed: "[::]"
        if(!parseIPv6Host(uri, idx)) return false;
    }
    else if(!parseHost(uri, idx)) return false;
    
    // idx should either be pointing at ':' for port,
    // '/' for resource name, or the end of uri
    if(!parsePort(uri, idx)) return false;
    
    // store what is left into resourceName
    std::ostringstream os;
    while(idx < uri.length()) {
        if(uri[idx] == '#') break;
        os << uri[idx];
        idx++;
    }
    _resource_ = os.str();
    
    return true;
}

bool URI::parseScheme(std::string uri) {
    std::ostringstream os;
    const char diff = 'A' - 'a';
    unsigned int idx = 0, length = uri.length();
    bool foundDelimiter = false;
    
    while(idx < length) {
        if(uri[idx] == ':') {
            foundDelimiter = true;
            break;
        }
        else if(uri[idx] >= 'A' && uri[idx] <= 'Z') {
            // to lower
            os << (uri[idx] - diff);
        }
        else os << uri[idx];
        idx++;
    }
    
    _scheme_.assign(os.str());
    return foundDelimiter;
}

bool URI::parseIPv6Host(std::string uri, unsigned int &offset) {
    const int MAX_HOST  = 39; // IPv6 fully exapanded with ':' is 39 cahrs
    const int MAX_LABEL =  4; // labels are only 4 hex chars long
    const int MIN_LEN   =  4; // "[::]"
    if((uri.length() - offset) < MIN_LEN) return false;
    
    std::ostringstream os;
    int colonDelimiterCount = 0;
    unsigned int& idx = offset;
    
    if(uri[idx] != '[') return false; // double check
    os << uri[idx];
    idx++;
    int labelLen = 0, hostLen = 0;
    while(idx < uri.length()) {
        if(uri[idx] == '/') return false; // didn't close with ']'
        else if(uri[idx] == ':') {
            labelLen = 0;
            hostLen++;
            os << uri[idx];
            
            colonDelimiterCount++;
            // IPv6 doesn't have more than 8 groups
            if (colonDelimiterCount > 7) return false;
        }
        else if(uri[idx] == ']') {
            // expecting "::" minimum
            if(colonDelimiterCount < 2) return false;
            else {
                os << uri[idx];
                idx++; // align with parseHost() function's return index
                _host_ = os.str();
                return true;
            }
        }
        else if( // only allow legal hex values
            (uri[idx] >= 'a' && uri[idx] <= 'f') || // a-z
            (uri[idx] >= 'A' && uri[idx] <= 'F') || // A-Z
            (uri[idx] >= '0' && uri[idx] <= '9')
        ) {
            labelLen++;
            hostLen++;
            os << uri[idx];
        }
        else return false;
        
        if(labelLen > MAX_LABEL || hostLen > MAX_HOST) return false;
        idx++;
    }
    
    return false;
}

bool URI::parseHost(std::string uri, unsigned int &offset) {
    const int MAX_HOST  = 254; // 253 + '.'
    const int MAX_LABEL =  63;
    const int MIN_LEN   =   3; // "a.z"
    if((uri.length() - offset) < MIN_LEN) return false;
    
    std::ostringstream os;
    int labelLen = 0, hostLen = 0;
    bool firstChar = true;
    unsigned int& idx = offset;
    
    while(idx < uri.length()) {
        if(firstChar) {
            // RFC 952: non-alphanumeric chars as the first char
            // are not allowed.
            if(uri[idx] == '-') return false;
            else firstChar = false;
        }
        if(uri[idx] == '/') {
            if(hostLen < 1) return false;
            break;
        }
        else if(uri[idx] == ':') break;
        else if(
            (uri[idx] >= 'a' && uri[idx] <= 'z') || // a-z
            (uri[idx] >= 'A' && uri[idx] <= 'Z') || // A-Z
            (uri[idx] >= '0' && uri[idx] <= '9') || // 0-9
            (uri[idx] == '-')
        ) {
            os << uri[idx];
            labelLen++;
            hostLen++;
        }
        else if(uri[idx] == '.') {
            // end of label
            os << uri[idx];
            labelLen = 0;
            hostLen++;
        }
        else return false;
        
        if(labelLen > MAX_LABEL || hostLen > MAX_HOST) return false;
        idx++;
    }
    
    _host_ = os.str();
    return true;
}

bool URI::parsePort(std::string uri, unsigned int &offset) {
    unsigned int& idx = offset;
    std::ostringstream os;
    
    _port_ = 0;
    if(idx < uri.length() && uri[idx] == ':') {
        // parse port
        const int MAX_CHARS = 5; // "65535"
        int charCount = 0;
        idx++; // skip ':'
        while(idx < uri.length()) {
            if(uri[idx] == '/') break;
            else if (uri[idx] >= '0' && uri[idx] <= '9') {
                os << uri[idx];
                charCount++;
            }
            else return false;
            if(charCount > MAX_CHARS) return false;
            idx++;
        }
        
        std::string strport = os.str();
        if(strport.length() >= 1) {
            // only try to parse if there is something to parse
            _port_ = std::stoi(strport);
            if(_port_ > 65535) return false;
        }
    }
    
    // if port is still 0, attempt to resolve port based on scheme
    if(_port_ == 0) {
        if(_scheme_ == "http")       _port_ = PORT;
        else if(_scheme_ == "https") _port_ = SECURE_PORT;
    }
    return true;
}