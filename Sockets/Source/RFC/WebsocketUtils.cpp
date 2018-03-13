/**
 * Created by TekuConcept on March 8, 2018
 */

#include "RFC/WebsocketUtils.h"
#include "RFC/3174"
#include "RFC/4648"
#include "RFC/6455"
#include <sstream>

using namespace Impact;
using namespace Internal;

std::uniform_int_distribution<unsigned short>
WebsocketUtils::_distribution_(0,255);
std::string WebsocketUtils::SECRET = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

WSFrameContext::WSFrameContext() : finished(true), reserved(0), opcode(1),
    masked(false), length(0), mask_key{0x0,0x0,0x0,0x0}, _processed_(0),
    _continued_(false) {}


std::string WebsocketUtils::generateKey(std::mt19937 _engine_) {
    std::ostringstream os;
    for(int i = 0; i < 16; i++)
        os << (unsigned char)_distribution_(_engine_);
    return Base64::encode(os.str());
}


std::string WebsocketUtils::hashKey(std::string key) {
    std::string hash = key;
    hash.append(SECRET);
    return Base64::encode(SHA1::digest(hash));
}


std::string WebsocketUtils::getRequestKey(RFC2616::RequestMessage request) {
    const unsigned int KEY_SIZE = 24;
    
    if(request.method() != RFC2616::METHOD::GET)            return "";
    if(request.major() < 1 || request.minor() < 1)          return "";
    if(request.getHeaderValue(RFC6455::toString(
        RFC6455::HEADER::SecWebSocketVersion)) != "13")     return "";
    if(request.getHeaderValue(RFC2616::HEADER::Upgrade) !=
        RFC2616::string("websocket"))                       return "";
    if(request.getHeaderValue(RFC2616::HEADER::Connection) !=
        RFC2616::string("upgrade"))                         return "";
    if(request.getHeaderValue(RFC2616::HEADER::Host) == "") return "";
    
    std::string key = request.getHeaderValue(
                RFC6455::toString(RFC6455::HEADER::SecWebSocketKey));
    if(key.length() != KEY_SIZE)                            return "";
    return hashKey(key);
}


std::string WebsocketUtils::getResponseKey(RFC2616::ResponseMessage response) {
    if(response.status() != RFC2616::STATUS::SWITCHING)         return "";
    else if(response.getHeaderValue(
        RFC6455::toString(RFC6455::HEADER::SecWebSocketExtensions))
        .length() != 0) /* no extensions in this connecton */   return "";
    else if(response.getHeaderValue(
        RFC6455::toString(RFC6455::HEADER::SecWebSocketProtocol))
        .length() != 0) /* no special protocols used */         return "";
    // check key matches
    auto key = response.getHeaderValue(
        RFC6455::toString(RFC6455::HEADER::SecWebSocketAccept)
    );
    return key;
}


std::string WebsocketUtils::SYN(std::iostream& _stream_,
    URI _uri_, std::mt19937 _engine_) {
    std::string key    = generateKey(_engine_);
    
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
    message.addHeader(RFC6455::toString(RFC6455::HEADER::SecWebSocketKey), key);
    message.addHeader(RFC6455::toString(RFC6455::HEADER::SecWebSocketVersion),
        "13");
    
    _stream_ << message.toString() << std::flush;
    return hashKey(key);
}


std::string WebsocketUtils::SYNACK(std::iostream& _stream_) {
    using RFC2616::RequestMessage;
    using RFC2616::ResponseMessage;
    using RFC2616::STATUS;
    using RFC2616::HEADER;
    
    std::string key = "";
    bool check;
    RequestMessage request = RequestMessage::tryParse(_stream_, check);
    auto status = check ? STATUS::SWITCHING : STATUS::BAD_REQUEST;

    if(status == STATUS::SWITCHING) {
        key = getRequestKey(request);
        if(key.length() <= 0) status = STATUS::BAD_REQUEST;
    }
    
    ResponseMessage response(status);
    if(status == STATUS::SWITCHING) {
        response.addHeader(HEADER::Upgrade, "websocket");
        response.addHeader(HEADER::Connection, "Upgrade");
        response.addHeader(
            RFC6455::toString(RFC6455::HEADER::SecWebSocketAccept), key);
    }
    
    _stream_ << response.toString() << std::flush;
    return key;
}


bool WebsocketUtils::ACK(std::iostream& _stream_, std::string _key_) {
    using RFC2616::ResponseMessage;
    
    bool check, check2 = false;
    ResponseMessage response = ResponseMessage::tryParse(_stream_, check);
    if(check) check2 = (getResponseKey(response) == _key_);
    
    return check && check2;
}


/*
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-------+-+-------------+-------------------------------+
 |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
 |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
 |N|V|V|V|       |S|             |   (if payload len==126/127)   |
 | |1|2|3|       |K|             |                               |
 +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
 |     Extended payload length continued, if payload len == 127  |
 + - - - - - - - - - - - - - - - +-------------------------------+
 |                               |Masking-key, if MASK set to 1  |
 +-------------------------------+-------------------------------+
 | Masking-key (continued)       |          Payload Data         |
 +-------------------------------- - - - - - - - - - - - - - - - +
 :                     Payload Data continued ...                :
 + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 |                     Payload Data continued ...                |
 +---------------------------------------------------------------+
*/


bool WebsocketUtils::writeHeader(std::ostream& stream,
    WSFrameContext& header, std::mt19937 engine) {
    #define BAD_WRITE_CHECK(x) if(!(stream << x)) return false;
    
    BAD_WRITE_CHECK((unsigned char)(
        (header.finished?0x80:0x00)    |
        ((header.reserved & 0x7) << 4) |
        (header.opcode & 0xF)
    ))
    
    unsigned char preLength = (unsigned char)(header.masked?0x80:0x00);
    if(header.length <= 125)         preLength |= header.length;
    else if(header.length <= 0xFFFF) preLength |= 0x7E;
    else                             preLength |= 0x7F;
    BAD_WRITE_CHECK(preLength);
    preLength &= 0x7F;
    
    auto size = 0;
    if(preLength == 126) size = 2;
    else if(preLength == 127) size = 8;
    if(size > 0 && !(stream.write((const char*)&header.length,size)))
        return false;
    
    if(header.masked) {
        for(int i = 0; i < 4; i++) {
            header.mask_key[i] = (unsigned char)_distribution_(engine);
            BAD_WRITE_CHECK(header.mask_key[i])
        }
    }
    
    stream << std::flush;

    return true;
    #undef BAD_WRITE_CHECK
}


bool WebsocketUtils::writeData(std::ostream& stream, WSFrameContext context,
    const char* data, unsigned int length, int& keyOffset) {
    #define BAD_WRITE_CHECK(x) if(!(stream << x)) return false;
    
    for(unsigned int i = 0; i < length; i++) {
        BAD_WRITE_CHECK((unsigned char)(data[i]^context.mask_key[keyOffset]))
        if(i%256==0) stream << std::flush;
        keyOffset = (keyOffset+1)%4;
    }
    stream << std::flush;
    
    return true;
    #undef BAD_WRITE_CHECK
}


bool WebsocketUtils::readHeader(std::istream& stream, WSFrameContext& header) {
    #define EOF_READ_CHECK(x) if(!(stream >> x)) return false;
    
    unsigned char byte;
    EOF_READ_CHECK(byte)
    header.finished = (byte >> 7);
    header.reserved = (byte >> 4) & 0x7;
    header.opcode   = byte & 0xF;
    
    EOF_READ_CHECK(byte)
    header.masked   = (byte >> 7);
    unsigned char preLength  = (unsigned char)(byte & 0x7F);
    
    if(preLength <= 125) header.length = preLength;
    else if(preLength == 126) {
        unsigned char msb, lsb;
        EOF_READ_CHECK(msb)
        EOF_READ_CHECK(lsb)
        header.length = (msb << 8) | lsb;
    }
    else {
        header.length = 0;
        for(auto i = 0; i < 8; i++) {
            EOF_READ_CHECK(byte)
            header.length = (header.length<<8) | byte;
        }
    }
    
    for(auto i = 0; i < 4; i++) {
        if(header.masked) { EOF_READ_CHECK(header.mask_key[i]) }
        else header.mask_key[i] = 0x00;
    }
    
    return true;
    #undef EOF_READ_CHECK
}


int WebsocketUtils::readData(std::istream& stream, WSFrameContext context,
    char* data, int length, int& keyOffset) {
    #define EOF_READ_CHECK(x) if(!stream.get(x)) return count;
    if(length < 0) return -1;
    else if (length == 0) return 0;
    int count = 0;
    
    char byte;
    for(int i = 0; i < length; i++) {
        EOF_READ_CHECK(byte);
        data[i] = (char)((int)(0xFF&byte) ^ context.mask_key[keyOffset]);
        count++;
        keyOffset = (keyOffset+1)%4;
    }
    
    return count;
    #undef EOF_READ_CHECK
}