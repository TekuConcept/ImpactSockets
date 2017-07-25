/**
 * Created by TekuConcept on July 21, 2017
 */

#include "RFC/RFC6455.h"
#include <sstream>

using namespace Impact;

void RFC6455::URI::DerefFragment(std::string &uri) {
    std::ostringstream ss;
    const char* buffer = uri.c_str();
    for(unsigned int i = 0; i < uri.length(); i++) {
        if(buffer[i] == '#') break;
        else                 ss << buffer[i];
    }
    uri.assign(ss.str());
}

void RFC6455::URI::EscapeAllPound(std::string &uri) {
    std::ostringstream ss;
    const char* buffer = uri.c_str();
    for(unsigned int i = 0; i < uri.length(); i++) {
        if(buffer[i] == '#') ss << "%%23";
        else                 ss << buffer[i];
    }
    uri.assign(ss.str());
}

std::string RFC6455::getRequestMessage(RFC2616::URI uri) {
    RFC2616::RequestMessage message(
        RFC2616::Request::METHOD::GET,
        uri.resource()
    );
    
    if(uri.port() != RFC2616::PORT && uri.port() != RFC2616::SECURE_PORT) {
        std::ostringstream os;
        os << uri.host() << ":" << uri.port();
        message.addHeader(RFC2616::HEADER::Host, os.str());
    }
    else message.addHeader(RFC2616::HEADER::Host, uri.host());
    
    message.addHeader(RFC2616::HEADER::Upgrade, "websocket");
    message.addHeader(RFC2616::HEADER::Connection, "upgrade");
    
    return message.toString();
}