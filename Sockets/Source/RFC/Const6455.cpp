/**
 * Created by TekuConcept on July 27, 2017
 */

#include "RFC/Const6455.h"
#include <map>

using namespace Impact;

typedef std::pair<unsigned int, std::string> Token;
typedef std::pair<RFC2616::string, unsigned int> Token2;

std::map <unsigned int, std::string> HEADER_NAMES_6455 = {
    Token(0, "Sec-WebSocket-Key"),
    Token(1, "Sec-WebSocket-Extensions"),
    Token(2, "Sec-WebSocket-Accept"),
    Token(3, "Sec-WebSocket-Protocol"),
    Token(4, "Sec-WebSocket-Version")
};

std::map <RFC2616::string, unsigned int> HEADER_NAMES_REVERSE_LOOKUP_6455 = {
    Token2("Sec-WebSocket-Key", 0),
    Token2("Sec-WebSocket-Extensions", 1),
    Token2("Sec-WebSocket-Accept", 2),
    Token2("Sec-WebSocket-Protocol", 3),
    Token2("Sec-WebSocket-Version", 4)
};

bool RFC6455::findHeader(const std::string header, HEADER &code) {
    RFC2616::string temp(header.c_str(), header.length());
    return findHeader(temp, code);
}

bool RFC6455::findHeader(const RFC2616::string header, HEADER &code) {
    auto token = HEADER_NAMES_REVERSE_LOOKUP_6455.find(header);
    if(token != HEADER_NAMES_REVERSE_LOOKUP_6455.end()) {
        code = (HEADER)token->second;
        return true;
    }
    return false;
}

std::string RFC6455::toString(HEADER code) {
    return HEADER_NAMES_6455[(int)code];
}