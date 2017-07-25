/**
 * Created by TekuConcept on July 24, 2017
 */

#include "RFC/RequestMessage.h"
#include <sstream>

using namespace Impact;
using namespace RFC2616;

RequestMessage::RequestMessage(Request::METHOD method, std::string uri)
    : _method_(method) {
    if(uri.length() == 0) _uri_ = "/";
    else if(uri.at(0) == '*' && uri.length() != 1) _uri_ = "*";
    else _uri_ = uri;
}

void RequestMessage::addHeader(HEADER header, std::string value) {
    // check if header already exists - use map instead of vector
    _headers_.push_back(StringHeaderPair(header, value));
}

void RequestMessage::addUserHeader(std::string header, std::string value) {
    _userHeaders_.push_back(StringStringPair(header, value));
}

std::string RequestMessage::toString() {
    std::ostringstream os;
    os << RFC2616::toString(_method_) << SP << _uri_ << SP << HTTP_VERSION << CRLF;
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