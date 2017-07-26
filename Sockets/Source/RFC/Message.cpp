/**
 * Created by TekuConcept on July 25, 2017
 */

#include <RFC/Message.h>

using namespace Impact;
using namespace RFC2616;

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

void Message::addHeader(HEADER header, std::string value) {
    // check if header already exists - use map instead of vector
    _headers_.push_back(StringHeaderPair(header, value));
}

void Message::addUserHeader(std::string header, std::string value) {
    if(header.find(":") != std::string::npos) throw std::exception();
    _userHeaders_.push_back(StringStringPair(header, value));
}