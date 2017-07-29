/**
 * Created by TekuConcept on July 27, 2017
 */

#include "RFC/ResponseMessage.h"
#include "RFC/Websocket.h"
#include "RFC/Base64.h"
#include "RFC/SHA1.h"
#include <sstream>
#include <random>
#include <chrono>

#include <iostream>
#define DMSG(x) std::cerr << x << std::endl

using namespace Impact;
using namespace RFC6455;

Websocket::Websocket(std::iostream &stream)
    : _stream_(stream), SECRET("258EAFA5-E914-47DA-95CA-C5AB0DC85B11") {
    auto now = std::chrono::high_resolution_clock::now();
    _engine_.seed(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()
        ).count()
    );
}

void Websocket::initiateClientHandshake(WSURI uri) {
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
    std::string key = Base64::encode(os.str());
    std::string hash = key;
    hash.append(SECRET);
    _key_ = SHA1::digest(hash);
    return key;
}

bool Websocket::initiateServerHandshake() {
    using RFC2616::RequestMessage;
    using RFC2616::ResponseMessage;
    using RFC2616::STATUS;
    using RFC2616::HEADER;
    
    bool check;
    RequestMessage request = RequestMessage::tryParse(_stream_, check);

    std::string key, hash;
    STATUS status = check ? STATUS::SWITCHING : STATUS::BAD_REQUEST;

    if(status == STATUS::SWITCHING) {
        if(!validateRequest(request, key)) {
            status = STATUS::BAD_REQUEST;
        }
        else {
            key.append(SECRET);
            hash = Base64::encode(SHA1::digest(key));
        }
    }
    
    ResponseMessage response(status);
    if(status == STATUS::SWITCHING) {
        response.addHeader(HEADER::Upgrade, "websocket");
        response.addHeader(HEADER::Connection, "upgrade");
        response.addHeader(
            RFC6455::toString(RFC6455::HEADER::SecWebSocketAccept), hash);
    }
    
    _stream_ << response.toString();
    return status == STATUS::SWITCHING;
}

bool Websocket::validateRequest(RFC2616::RequestMessage request,
    std::string &key) {
    if(request.method() != RFC2616::METHOD::GET)            return false;
    if(request.major() < 1 || request.minor() < 1)          return false;
    if(request.getHeaderValue(RFC6455::toString(
        RFC6455::HEADER::SecWebSocketVersion)) != "13")     return false;
    if(request.getHeaderValue(RFC2616::HEADER::Upgrade) !=
        RFC2616::string("websocket"))                       return false;
    if(request.getHeaderValue(RFC2616::HEADER::Connection) !=
        RFC2616::string("upgrade"))                         return false;
    if(request.getHeaderValue(RFC2616::HEADER::Host) == "") return false;
    
    const unsigned int KEY_SIZE = 24;
    key = request.getHeaderValue(
                RFC6455::toString(RFC6455::HEADER::SecWebSocketKey));
    if(key.length() != KEY_SIZE)                            return false;
    
    return true;
}

bool Websocket::acceptResponse() {
    using RFC2616::ResponseMessage;
    
    bool check;
    ResponseMessage message = ResponseMessage::tryParse(_stream_, check);
    
    if(!check)                                                  return false;
    else if(message.status() != RFC2616::STATUS::SWITCHING)     return false;
    else if(message.getHeaderValue(
        RFC6455::toString(RFC6455::HEADER::SecWebSocketExtensions))
        .length() != 0) /* no extensions in this connecton */   return false;
    else if(message.getHeaderValue(
        RFC6455::toString(RFC6455::HEADER::SecWebSocketProtocol))
        .length() != 0) /* no special protocols used */         return false;
    else { // check key matches
        const unsigned int KEY_SIZE = 20;
        check = false;
        auto key = Base64::decode(
            message.getHeaderValue(
                RFC6455::toString(RFC6455::HEADER::SecWebSocketAccept)
            ),
            check
        );
        if(!check)                                              return false;
        else if(key.length() != KEY_SIZE)                       return false;
        else if(key != _key_)                                   return false;
    }
    
    return true;
}