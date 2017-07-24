/**
 * Created by TekuConcept on July 24, 2017
 */

#include "RFC/RequestMessage.h"
#include <sstream>

using namespace Impact;
using namespace RFC2616;
using namespace Request;

Message::Message(METHOD method, std::string uri)
    : _method_(method) {
    if(uri.length() == 0) _uri_ = "/";
    else if(uri.at(0) == '*' && uri.length() != 1) _uri_ = "*";
    else _uri_ = uri;
}

void Message::addHeader(HEADER header, std::string value) {
    (void)header;
    (void)value;
}

std::string Message::toString() {
    std::ostringstream os;
    os << RFC2616::toString(_method_) << SP << _uri_ << SP << HTTP_VERSION << CRLF;
    os << CRLF;
    return os.str();
}