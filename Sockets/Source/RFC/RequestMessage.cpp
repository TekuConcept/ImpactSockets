/**
 * Created by TekuConcept on July 24, 2017
 */

#include "RFC/RequestMessage.h"
#include <sstream>
#include <exception>

using namespace Impact;
using namespace RFC2616;

RequestMessage::RequestMessage() {}

RequestMessage::RequestMessage(Request::METHOD method, std::string uri)
    : _method_(method), _major_(HTTP_MAJOR), _minor_(HTTP_MINOR) {
    if(uri.length() == 0) _uri_ = "/";
    else if(uri[0] == '*' && uri.length() != 1) _uri_ = "*";
    else _uri_ = uri;
}

RequestMessage::RequestMessage(std::string message) {
    std::stringstream ss(message);
    if(!parse(ss)) throw std::exception();
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
    os << RFC2616::toString(_method_) << SP << _uri_ << SP
        << HTTP_PREFIX << _major_ << "." << _minor_ << CRLF;
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
    std::string requestHeader, optionalHeader;
    if (!getline(request, requestHeader))               return false;
    
    std::vector<std::string> pendingHeaders;
    unsigned int length;
    do {
        if (!getline(request, optionalHeader))          return false;
        length = optionalHeader.length();
        if (length == 0) return false;
        else if (optionalHeader[length - 1] != '\r')    return false;
        else if (length != 1) {
            pendingHeaders.push_back(optionalHeader.substr(0, length-1));
        }
    } while(length > 1);

    if(requestHeader.length() < MIN_REQUEST_HEADER_LEN) return false;
    else if(!parseRequestHeader(requestHeader))         return false;
    
    // TODO: parse headers
    // if(!parseOptionalHeaders(pendingHeaders) return false;
    
    // todo get body?
    
    return true;
}

bool RequestMessage::parseRequestHeader(std::string header) {
    if(!parseRequestMethod(header)) return false;
    unsigned int idx = RFC2616::toString(_method_).length();

    while(RFC2616::isWhiteSpace(header[idx])) {
        idx++; // skip LWS
        if((header.length() - idx) < 10) return false; // "/ HTTP/1.1"
    }
    
    if(!parseRequestURI(header, idx)) return false;
    
    while(RFC2616::isWhiteSpace(header[idx])) {
        idx++; // skip LWS
        if((header.length() - idx) < 8) return false; // "HTTP/1.1"
    }
    
    if(!parseRequestVersion(header, idx)) return false;
    return true;
}

bool RequestMessage::parseRequestMethod(std::string header) {
    switch(header[0]) {
        case 'C': _method_ = RFC2616::Request::METHOD::CONNECT; break;
        case 'D': _method_ = RFC2616::Request::METHOD::DELETE;  break;
        case 'G': _method_ = RFC2616::Request::METHOD::GET;     break;
        case 'H': _method_ = RFC2616::Request::METHOD::HEAD;    break;
        case 'O': _method_ = RFC2616::Request::METHOD::OPTIONS; break;
        case 'P':
            if(header[1] == 'O')
                _method_ = RFC2616::Request::METHOD::POST;
            else if(header[1] == 'U')
                _method_ = RFC2616::Request::METHOD::PUT;
            else return false;
            break;
        case 'T': _method_ = RFC2616::Request::METHOD::TRACE;   break;
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

bool RequestMessage::parseRequestVersion(std::string header, unsigned int &offset) {
    std::string version = HTTP_PREFIX;
    unsigned int &idx = offset;
    
    // check protocol identifier
    for(unsigned int i = 0, len = version.length(); i < len; i++) {
        if(version[i] != header[idx]) return false;
        idx++;
    }
    
    // get major
    std::ostringstream os;
    while(idx < header.length()) {
        if(header[idx] >= '0' && header[idx] <= '9')
            os << header[idx];
        else if (header[idx] == '.') {
            idx++;
            break;
        }
        else return false;
        idx++;
    }
    _major_ = std::stoi(os.str());
    
    // get minor
    os.str(std::string());
    while(idx < header.length()) {
        if(header[idx] >= '0' && header[idx] <= '9')
            os << header[idx];
        else if (header[idx] == RFC2616::CRLF[0] ||
            RFC2616::isWhiteSpace(header[idx])) {
            // end of line found, break
            break;
        }
        else return false;
        idx++;
    }
    _minor_ = std::stoi(os.str());

    return true;
}

Request::METHOD RequestMessage::method() {
    return _method_;
}

std::string RequestMessage::resource() {
    return _uri_;
}

unsigned int RequestMessage::major() {
    return _major_;
}

unsigned int RequestMessage::minor() {
    return _minor_;
}