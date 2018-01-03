/**
 * Created by TekuConcept on July 28, 2017
 */

#include "RFC/WebsocketServerNode.h"
#include "RFC/ResponseMessage.h"
#include "RFC/Const6455.h"
#include "RFC/Base64.h"
#include "RFC/SHA1.h"

#define DMSG(x) std::cerr << x << std::endl

using namespace Impact;
using namespace RFC6455;

WebsocketServerNode::WebsocketServerNode(std::iostream &stream)
    : Websocket(stream, false) {}

bool WebsocketServerNode::initiateHandshake() {
    Websocket::initiateHandshake();
    using RFC2616::RequestMessage;
    using RFC2616::ResponseMessage;
    using RFC2616::STATUS;
    using RFC2616::HEADER;
    
    bool check;
    RequestMessage request = RequestMessage::tryParse(_stream_, check);

    std::string hash;
    STATUS status = check ? STATUS::SWITCHING : STATUS::BAD_REQUEST;

    if(status == STATUS::SWITCHING) {
        if(!validateRequest(request)) status = STATUS::BAD_REQUEST;
        else hash = Base64::encode(_key_);
    }
    
    ResponseMessage response(status);
    if(status == STATUS::SWITCHING) {
        response.addHeader(HEADER::Upgrade, "websocket");
        response.addHeader(HEADER::Connection, "Upgrade");
        response.addHeader(
            RFC6455::toString(RFC6455::HEADER::SecWebSocketAccept), hash);
    }
    
    _stream_ << response.toString() << std::flush;
    if(status == STATUS::SWITCHING) {
        _connectionState_ = STATE::OPEN;
        return true;
    }
    else {
        _connectionState_ = STATE::CLOSED;
        return false;
    }
}

bool WebsocketServerNode::validateRequest(RFC2616::RequestMessage request) {
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
    std::string key = request.getHeaderValue(
                RFC6455::toString(RFC6455::HEADER::SecWebSocketKey));
    if(key.length() != KEY_SIZE)                            return false;
    else {
        key.append(SECRET);
        _key_ = SHA1::digest(key);
    }
    
    return true;
}