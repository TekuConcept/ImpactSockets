/**
 * Created by TekuConcept on July 25, 2017
 * 
 * URIs should be immutable objects.
 * Its properties should be readonly.
 */

#include "RFC/URI.h"
#include "RFC/Const2616.h"
#include <sstream>
#include <exception>
#include <stdexcept>
#include <algorithm>

#include <iostream>
#define DMSG(x) std::cerr << x << std::endl

using namespace Impact;
using namespace RFC2616;

typedef std::pair<std::string, std::pair<bool, unsigned short>> MetaToken;
typedef std::pair<bool, unsigned short> MetaValue;

std::map<std::string, std::pair<bool,unsigned short>> URI::_schemeMetaData_
= {
    MetaToken("http",  MetaValue(false, 80)),
    MetaToken("https", MetaValue(true, 443)),
    MetaToken("ws",    MetaValue(false, 80)),
    MetaToken("wss",   MetaValue(true, 443))
};

URI::URI() {}

URI::URI(std::string uri) {
    if(!parse(uri)) throw std::runtime_error("Bad URI");
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

unsigned short URI::port() {
    return _port_;
}

bool URI::secure() {
    return _secure_;
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
        if(uri[idx] == SP) os << "%%20";
        else os << uri[idx];
        idx++;
    }
    _resource_ = os.str();
    if(_resource_.length() == 0) _resource_ = "/";
    
    return true;
}

bool URI::parseScheme(std::string uri) {
    std::ostringstream os;
    unsigned int idx = 0, length = uri.length();
    bool foundDelimiter = false;
    
    while(idx < length) {
        if(uri[idx] == ':') {
            foundDelimiter = true;
            break;
        }
        else os << RFC2616::toLower(uri[idx]);
        idx++;
    }
    
    _scheme_.assign(os.str());
    
    setMetaInfo();
    
    return foundDelimiter;
}

void URI::setMetaInfo() {
    auto meta = _schemeMetaData_.find(_scheme_);
    if(meta != _schemeMetaData_.end()) {
        _secure_ = meta->second.first;
        _port_   = meta->second.second;
    }
    else {
        _secure_ = false;
        _port_   = 0;
    }
}

bool URI::parseIPv6Host(std::string uri, unsigned int &offset) {
    const int MAX_HOST  = 39; // IPv6 fully exapanded with ':' is 39 chars
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
        if(uri[idx] == '/') break; // didn't close with ']'
        else if(uri[idx] == ':') {
            labelLen = 0;
            hostLen++;
            os << uri[idx];
            
            colonDelimiterCount++;
            // IPv6 doesn't have more than 8 groups
            if (colonDelimiterCount > 7) break;
        }
        else if(uri[idx] == ']') {
            // expecting "::" minimum
            if(colonDelimiterCount < 2) break;
            else {
                os << uri[idx];
                idx++; // align with parseHost() function's return index
                _host_ = os.str();
                return true;
            }
        }
        else if( // only allow legal hex values
            (uri[idx] >= 'a' && uri[idx] <= 'f') || // a-f
            (uri[idx] >= 'A' && uri[idx] <= 'F') || // A-F
            (uri[idx] >= '0' && uri[idx] <= '9')) {
            labelLen++;
            hostLen++;
            os << RFC2616::toLower(uri[idx]);
        }
        else break;
        
        if(labelLen > MAX_LABEL || hostLen > MAX_HOST) break;
        idx++;
    }
    
    return false;
}

bool URI::parseHost(std::string uri, unsigned int &offset) {
    const int MAX_HOST  = 254; // 253 + '.'
    const int MAX_LABEL =  63;
    const int MIN_LEN   =   1; // "a"
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
            (uri[idx] >= 'a' && uri[idx] <= 'z') ||
            (uri[idx] >= 'A' && uri[idx] <= 'Z') ||
            (uri[idx] >= '0' && uri[idx] <= '9') || // 0-9
            (uri[idx] == '-')) {
            os << RFC2616::toLower(uri[idx]);
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
            unsigned int tport = std::stoi(strport);
            if(tport > 65535) return false;
            else _port_ = static_cast<unsigned short>(tport);
        }
    }

    return true;
}

void URI::registerScheme(std::string name, unsigned short port, bool secure) {
    if(name.length() == 0) return;
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    _schemeMetaData_[name] = MetaValue(secure, port);
}