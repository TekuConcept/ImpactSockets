/**
 * Created by TekuConcept on July 22, 2017
 */

#include <map>
#include <sstream>
#include "RFC/RFC2616.h"

#include <iostream>
#define DMSG(x) std::cerr << x << std::endl

using namespace Impact;

bool parseRequestMethod(std::string request, RFC2616::Request::Info &info) {
    switch(request.at(0)) {
        case 'C': info.method = RFC2616::Request::METHOD::CONNECT; break;
        case 'D': info.method = RFC2616::Request::METHOD::DELETE;  break;
        case 'G': info.method = RFC2616::Request::METHOD::GET;     break;
        case 'H': info.method = RFC2616::Request::METHOD::HEAD;    break;
        case 'O': info.method = RFC2616::Request::METHOD::OPTIONS; break;
        case 'P':
            if(request.at(1) == 'O')
                info.method = RFC2616::Request::METHOD::POST;
            else if(request.at(1) == 'U')
                info.method = RFC2616::Request::METHOD::PUT;
            else return false;
            break;
        case 'T': info.method = RFC2616::Request::METHOD::TRACE;   break;
        default: return false;
    }
    // double check because we only initially checked the first character(s)
    if(request.find(RFC2616::toString(info.method)) != 0)
        return false;
    return true;
}

bool parseRequestURI(const char* buffer, unsigned int length,
    unsigned int &idx, RFC2616::Request::Info &info) {
    std::ostringstream os;
    if(buffer[idx] == '*' && buffer[idx+1] != ' ') return false;
    else if(buffer[idx] != '/') return false; // full paths not yet supported
    
    while(idx < length) {
        if(buffer[idx] == ' ') break;
        else os << buffer[idx];
        idx++;
    }
    info.requestURI = os.str();
    return true;
}

bool parseRequestVersion(const char* buffer, unsigned int length,
    unsigned int &idx, RFC2616::Request::Info &info) {
    const char* version = "HTTP/";
    
    // check protocol identifier
    for(unsigned int i = 0, len = 5; i < len; i++) {
        if(version[i] != buffer[idx]) return false;
        idx++;
    }
    
    // get major and minor
    std::ostringstream os;
    while(idx < length) {
        if(buffer[idx] >= '0' && buffer[idx] <= '9')
            os << buffer[idx];
        else if (buffer[idx] == '.') {
            idx++;
            break;
        }
        else return false;
        idx++;
    }
    info.major = std::stoi(os.str());
    os.str(std::string());
    while(idx < length) {
        if(buffer[idx] >= '0' && buffer[idx] <= '9')
            os << buffer[idx];
        else if (buffer[idx] == '\r') {
            // end of line found, break
            idx++;
            break;
        }
        else if (buffer[idx] == ' ' || buffer[idx] == '\t') {
            // skip whitespace
            idx++;
            continue;
        }
        else return false;
        idx++;
    }
    info.minor = std::stoi(os.str());
    return true;
}

bool parseRequestHeader(std::string header, RFC2616::Request::Info &info) {
    if(!parseRequestMethod(header, info)) return false;

    unsigned int idx    = RFC2616::toString(info.method).length(),
                 length = header.length();
    const char*  buffer = header.c_str();
    
    while(buffer[idx] == ' ' || buffer[idx] == '\t') {
        idx++; // skip LWS
        if((length - idx) < 10) return false; // "/ HTTP/1.1"
    }
    
    if(!parseRequestURI(buffer, length, idx, info))
        return false;
    
    while(buffer[idx] == ' ' || buffer[idx] == '\t') {
        idx++; // skip LWS
        if((length - idx) < 8) return false; // "HTTP/1.1"
    }
    
    if(!parseRequestVersion(buffer, length, idx, info)) return false;
    return true;
}

bool RFC2616::Request::parse(std::istream &request, Info &info) {
    const unsigned int MIN_REQUEST_HEADER_LEN = 14; // "GET / HTTP/1.X"
    std::string requestHeader, optionalHeader;
    if (!getline(request, requestHeader)) return false;
    
    unsigned int length;
    do {
        if (!getline(request, optionalHeader)) return false;
        length = optionalHeader.length();
        if (length == 0) return false;
        else if (optionalHeader[length - 1] != '\r') return false;
        else if (length != 1)
            info.headers.push_back(optionalHeader.substr(0, length-1));
    } while(length > 1);

    if(requestHeader.length() < MIN_REQUEST_HEADER_LEN) return false;
    else if(!parseRequestHeader(requestHeader, info)) return false;
    
    // todo validate headers?
    // todo get body?
    
    return true;
}

bool RFC2616::Request::validate(std::string request) {
    Info info;
    std::stringstream ss(request);
    // validate resource uri ?
    return parse(ss, info);
}