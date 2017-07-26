/**
 * Created by TekuConcept on July 25, 2017
 */

#include "RFC/Message.h"
#include <sstream>

using namespace Impact;
using namespace RFC2616;

Message::Message() : _major_(HTTP_MAJOR), _minor_(HTTP_MINOR) {}

bool Message::parse(std::istream &request) {
    std::string optionalHeader;
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
    
    // parse pending headers
    
    // RFC 7230 Section 3.2.4 Paragraph 4: Field value folding is obsolete.
    // White space between start line and first header must be rejected or ignored
    return true;
}

bool Message::parseVersion(std::string header, unsigned int &offset) {
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

void Message::addHeader(HEADER header, std::string value) {
    // check if header already exists - use map instead of vector
    _headers_.push_back(StringHeaderPair(header, value));
}

void Message::addUserHeader(std::string header, std::string value) {
    if(header.find(":") != std::string::npos) throw std::exception();
    _userHeaders_.push_back(StringStringPair(header, value));
}

unsigned int Message::major() {
    return _major_;
}

unsigned int Message::minor() {
    return _minor_;
}