/**
 * Created by TekuConcept on July 27, 2017
 */

#include "RFC/Websocket.h"
#include "RFC/RequestMessage.h"
#include "RFC/Const6455.h"
#include "RFC/Base64.h"
#include <sstream>
#include <random>
#include <chrono>

using namespace Impact;
using namespace RFC6455;

Websocket::Websocket(std::iostream &stream) : _stream_(stream) {
    auto now = std::chrono::high_resolution_clock::now();
    _engine_.seed(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()
        ).count()
    );
}

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
    message.addHeader(RFC6455::toString(RFC6455::HEADER::SecWebSocketKey),
        generateKey());
    message.addHeader(RFC6455::toString(RFC6455::HEADER::SecWebSocketVersion),
        "13");
    
    return message.toString();
}

std::string Websocket::generateKey() {
    std::ostringstream os;
    for(int i = 0; i < 16; i++)
        os << (unsigned char)_distribution_(_engine_);
    return RFC4648::Base64::encode(os.str());
}