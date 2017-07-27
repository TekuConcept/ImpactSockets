/**
 * Created by TekuConcept on July 24, 2017
 */

#include "RFC/RequestMessage.h"
#include <sstream>
#include <exception>

#include <iostream>
#define DMSG(x) std::cerr << x << std::endl

using namespace Impact;
using namespace RFC2616;

RequestMessage::RequestMessage() {}

RequestMessage::RequestMessage(METHOD method, std::string uri)
    : _method_(method) {
    if(uri.length() == 0) _uri_ = "/";
    else if(uri[0] == '*' && uri.length() != 1) _uri_ = "*";
    else _uri_ = uri;
}

RequestMessage::RequestMessage(std::string message) {
    std::stringstream ss(message);
    if(!parse(ss)) throw std::exception();
}

std::string RequestMessage::toString() {
    std::ostringstream os;
    os << RFC2616::toString(_method_) << SP << _uri_ << SP
        << HTTP_PREFIX << _major_ << "." << _minor_ << CRLF;
        
    // iterate through elements rather than increment
    for(auto header : _headers_) {
        os << RFC2616::toString(header.first);
        os << ":" << SP;
        os << header.second;
        os << CRLF;
    }
    for(auto header : _userHeaders_) {
        os << header.first.c_str();
        os << ":" << SP;
        os << header.second;
        os << CRLF;
    }
    
    os << CRLF;
    return os.str();
}

bool RequestMessage::validate(std::string request) {
    RequestMessage message;
    std::stringstream ss(request);
    return message.parse(ss);
}

RequestMessage RequestMessage::tryParse(std::istream &request,
    bool &success) {
    RequestMessage message;
    success = message.parse(request);
    return message;
}

bool RequestMessage::parse(std::istream &request) {
    const unsigned int MIN_REQUEST_HEADER_LEN = 14; // "GET / HTTP/1.X"
    std::string startHeader;
    
    if (!getline(request, startHeader))                return false;
    if (startHeader.length() < MIN_REQUEST_HEADER_LEN) return false;
    else if (!parseRequestHeader(startHeader))         return false;
    
    if (!Message::parse(request))                      return false;

    return true;
}

bool RequestMessage::parseRequestHeader(std::string header) {
    if(!parseRequestMethod(header)) return false;
    unsigned int idx = RFC2616::toString(_method_).length();

    // RFC 7230 Section 3.1.1: Only one space.
    if(header[idx] != SP) return false;
    else idx++;
    if(header[idx] == SP) return false;
    
    if(!parseRequestURI(header, idx)) return false;
    
    if(header[idx] != SP) return false;
    else idx++;
    if(header[idx] == SP) return false;
    
    if(!parseVersion(header, idx)) return false;
    return true;
}

bool RequestMessage::parseRequestMethod(std::string header) {
    switch(header[0]) {
        case 'C': _method_ = RFC2616::METHOD::CONNECT; break;
        case 'D': _method_ = RFC2616::METHOD::DELETE;  break;
        case 'G': _method_ = RFC2616::METHOD::GET;     break;
        case 'H': _method_ = RFC2616::METHOD::HEAD;    break;
        case 'O': _method_ = RFC2616::METHOD::OPTIONS; break;
        case 'P':
            if(header[1] == 'O')
                _method_ = RFC2616::METHOD::POST;
            else if(header[1] == 'U')
                _method_ = RFC2616::METHOD::PUT;
            else return false;
            break;
        case 'T': _method_ = RFC2616::METHOD::TRACE;   break;
        default: return false;
    }
    // double check because we only initially checked the first character(s)
    if(header.find(RFC2616::toString(_method_)) != 0) return false;
    else return true;
}

bool RequestMessage::parseRequestURI(std::string header, unsigned int &offset) {
    std::ostringstream os;
    unsigned int &idx = offset;
    
    if(header[idx] == '*' && !RFC2616::isWhiteSpace(header[idx+1]))
        return false;
    else if(header[idx] != '/') return false; // full paths not yet supported
    
    while(idx < header.length()) {
        if(RFC2616::isWhiteSpace(header[idx])) break;
        else os << header[idx];
        idx++;
    }
    _uri_ = os.str();
    return true;
}

METHOD RequestMessage::method() {
    return _method_;
}

std::string RequestMessage::resource() {
    return _uri_;
}