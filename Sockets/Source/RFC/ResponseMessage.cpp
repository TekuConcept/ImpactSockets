/**
 * Create by TekuConcept on July 25, 2017
 */

#include "RFC/ResponseMessage.h"
#include <sstream>

#include <iostream>
#define DMSG(x) std::cerr << x << std::endl

using namespace Impact;
using namespace RFC2616;

ResponseMessage::ResponseMessage() {}

ResponseMessage::ResponseMessage(STATUS code) : _status_(code) {}

std::string ResponseMessage::toString() {
    std::ostringstream os;
    os << HTTP_PREFIX << HTTP_MAJOR << "." << HTTP_MINOR << SP
        << (int)_status_ << SP << RFC2616::toString(_status_) << CRLF;
    
    // iterate through elements rather than increment
    for(unsigned int i = 0; i < _headers_.size(); i++) {
        os << RFC2616::toString(_headers_[i].first);
        os << ":" << SP;
        os << _headers_[i].second;
        os << CRLF;
    }
    for(unsigned int i = 0; i < _userHeaders_.size(); i++) {
        os << _userHeaders_[i].first;
        os << ":" << SP;
        os << _userHeaders_[i].second;
        os << CRLF;
    }
    
    os << CRLF;
    return os.str();
}

STATUS ResponseMessage::status() {
    return _status_;
}

ResponseMessage ResponseMessage::tryParse(std::istream &request,
    bool &success) {
    ResponseMessage message;
    success = message.parse(request);
    return message;
}

bool ResponseMessage::validate(std::string request) {
    ResponseMessage message;
    std::stringstream ss(request);
    return message.parse(ss);
}

bool ResponseMessage::parse(std::istream &response) {
    const unsigned int MIN_RESPONSE_HEADER_LEN = 16; // "HTTP/1.1 200 OK"
    std::string startHeader;
    
    if (!getline(response, startHeader))                return false;
    if (startHeader.length() < MIN_RESPONSE_HEADER_LEN) return false;
    else if (!parseResponseHeader(startHeader))         return false;
    
    if (!Message::parse(response))                      return false;

    return true;
}

bool ResponseMessage::parseResponseHeader(std::string header) {
    unsigned int idx = 0;
    if(RFC2616::isWhiteSpace(header[idx])) return false;
    if(!Message::parseVersion(header, idx)) return false;
    
    if((header.length() - idx) < 6) return false;
    
    if(header[idx] != SP) return false;
    else idx++;
    if(header[idx] == SP) return false;
    
    if(!parseStatusCode(header, idx)) return false;
    
    if(header[idx] != SP) return false;
    else idx++;
    if(header[idx] == SP) return false;
    
    // ignore status message
    
    return true;
}

bool ResponseMessage::parseStatusCode(std::string header, unsigned int &offset) {
    std::ostringstream os;
    const unsigned int DIGIT_COUNT = 3;
    unsigned int digitCount = 0;
    unsigned int &idx = offset;
    
    while(idx < header.length()) {
        if(header[idx] >= '0' && header[idx] <= '9') {
            os << header[idx];
            digitCount++;
            if(digitCount > DIGIT_COUNT) break;
        }
        else if(RFC2616::isWhiteSpace(header[idx])) break;
        else return false;
        idx++;
    }
    if(digitCount != DIGIT_COUNT) return false;
    
    int code = std::stoi(os.str());
    if(!RFC2616::validStatusCode(code)) return false;
    else _status_ = (STATUS)code;
    
    return true;
}