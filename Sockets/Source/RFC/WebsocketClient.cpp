/**
 * Created by TekuConcept on July 27, 2017
 */

#include "RFC/WebsocketClient.h"
#include "RFC/ResponseMessage.h"
#include "RFC/RequestMessage.h"
#include "RFC/Const6455.h"
#include "RFC/Base64.h"
#include "RFC/SHA1.h"
#include <sstream>
#include <random>
#include <chrono>

#include <iostream>
#define DMSG(x) std::cerr << x << std::endl

using namespace Impact;
using namespace RFC6455;

WebsocketClient::WebsocketClient(std::iostream &stream, WSURI uri)
    : _stream_(stream), _uri_(uri) {
    auto now = std::chrono::high_resolution_clock::now();
    _engine_.seed(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()
        ).count()
    );
}

bool WebsocketClient::initiateHandshake() {
    RFC2616::RequestMessage message(
        RFC2616::METHOD::GET,
        _uri_.resource()
    );
    
    if(_uri_.port() != RFC2616::PORT && _uri_.port() != RFC2616::SECURE_PORT) {
        std::ostringstream os;
        os << _uri_.host() << ":" << _uri_.port();
        message.addHeader(RFC2616::HEADER::Host, os.str());
    }
    else message.addHeader(RFC2616::HEADER::Host, _uri_.host());
    
    message.addHeader(RFC2616::HEADER::Upgrade, "websocket");
    message.addHeader(RFC2616::HEADER::Connection, "upgrade");
    message.addHeader(RFC6455::toString(RFC6455::HEADER::SecWebSocketKey),
        generateKey());
    message.addHeader(RFC6455::toString(RFC6455::HEADER::SecWebSocketVersion),
        "13");
    
    _stream_ << message.toString();
    return true;
}

std::string WebsocketClient::generateKey() {
    std::ostringstream os;
    for(int i = 0; i < 16; i++)
        os << (unsigned char)_distribution_(_engine_);
    std::string key = Base64::encode(os.str());
    std::string hash = key;
    hash.append(SECRET);
    _key_ = SHA1::digest(hash);
    return key;
}

bool WebsocketClient::acceptResponse() {
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