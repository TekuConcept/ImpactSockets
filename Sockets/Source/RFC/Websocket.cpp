/**
 * Created by TekuConcept on July 27, 2017
 */

#include "RFC/Websocket.h"
#include "RFC/RequestMessage.h"
#include <sstream>

using namespace Impact;
using namespace RFC6455;

Websocket::Websocket(std::iostream &stream) : _stream_(stream) {}

void Websocket::initiateHandshake(WSURI uri) {
    _stream_ << generateRequest(uri);
}

std::string Websocket::generateRequest(WSURI uri) {
    RFC2616::RequestMessage message(
        RFC2616::METHOD::GET,
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