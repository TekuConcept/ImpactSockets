/**
 * Created by TekuConcept on July 29, 2017
 */

#include "RFC/Websocket.h"
#include <chrono>

using namespace Impact;
using namespace RFC6455;

#define OP_CONTINUE 0
#define OP_TEXT     1
#define OP_BINARY   2
#define OP_CLOSE    8
#define OP_PING     9
#define OP_PONG    10

Websocket::Websocket(std::iostream& stream, bool isClient)
    : _stream_(stream), _connectionState_(STATE::CLOSED),
    _isClient_(isClient) {
    auto now = std::chrono::high_resolution_clock::now();
    _engine_.seed(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()
        ).count() + (int)this
        // 'this' prevents two instances having the same seed
    );
}

Websocket::~Websocket() {}

bool Websocket::initiateHandshake() {
    _connectionState_ = STATE::CONNECTING;
    return true;
}

void Websocket::ping() {
    DataFrame frame;
    initFrame(frame);
    frame.opcode     = OP_PING;
    serializeOut(frame);
}

void Websocket::pong() {
    DataFrame frame;
    initFrame(frame);
    frame.opcode     = OP_PONG;
    serializeOut(frame);
}

void Websocket::close() {
    DataFrame frame;
    initFrame(frame);
    frame.opcode     = OP_CLOSE;
    serializeOut(frame);
    _connectionState_ = STATE::CLOSED;
}

void Websocket::sendText(std::string text) {
    DataFrame frame;
    initFrame(frame);
    frame.opcode     = OP_TEXT;
    frame.length     = text.length();
    frame.data       = text.c_str();
    serializeOut(frame);
}

void Websocket::sendBinary(const char* data, unsigned int length) {
    DataFrame frame;
    initFrame(frame);
    frame.opcode     = OP_BINARY;
    frame.length     = length;
    frame.data       = data;
    serializeOut(frame);
}

void Websocket::initFrame(DataFrame &frame) {
    frame.finished   = true;
    frame.reserved   = '\0';
    frame.masked     = _isClient_;
    frame.length     = 0;
}

DataFrame Websocket::read() {
    DataFrame result = serializeIn();
    if(!(_isClient_ ^ result.masked)) {
        close();
        _connectionState_ = STATE::CLOSED;
    }
    else {
        switch(result.opcode) {
        case OP_PING: pong(); break; // TODO: identical application data
        case OP_CLOSE: if(_connectionState_ != STATE::CLOSED) close(); break;
        }
    }
    return result;
}

STATE Websocket::getState() {
    return _connectionState_;
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

void Websocket::serializeOut(DataFrame frame) {
    if(_connectionState_ == STATE::CLOSED) return;
    
    _stream_ << (unsigned char)((frame.finished ? 0x80 : 0x0) |
                               ((frame.reserved & 0x7) << 4)  | 
                                (frame.opcode   & 0xF));
    
    unsigned char l;
    if(frame.length <= 125)
         l = (unsigned char)(frame.length);
    else if (frame.length <= 65535)
         l = (unsigned char)126;
    else l = (unsigned char)127;
    _stream_ << (unsigned char)((frame.masked << 7) | l);
    
    if(frame.length >= 126 && frame.length <= 65535) {
        _stream_ << (char)(frame.length >> 8);
        _stream_ << (char)(frame.length & 0xFF);
    }
    else if (frame.length > 65535) {
        // TODO: MSb must be 0
        for(unsigned short i = 8; i <= 64; i+=8)
            _stream_ << (char)((frame.length>>(64-i)) & 0xFF);
    }
    
    unsigned char maskKey[4];
    for(unsigned short i = 0; i < 4; i++) {
        if(frame.masked) {
            maskKey[i] = (unsigned char)_distribution_(_engine_);
            _stream_ << maskKey[i];
        }
        else maskKey[i] = '\0';
    }
    for(uint64_t i = 0; i < frame.length; i++)
        _stream_ << (char)(frame.data[i]^maskKey[i%4]);
}

DataFrame Websocket::serializeIn() {
    DataFrame frame;
    unsigned char c;
    
    _stream_ >> c;
    frame.finished = ((c & 0x80) == 0x80);
    frame.reserved = ((c >> 4) & 0x7);
    frame.opcode   = (c & 0xF);
    
    _stream_ >> c;
    frame.masked         = ((c & 0x80) == 0x80);
    unsigned char length = (c & 0x7F);
    if(length <= 125)
        frame.length     = length;
    else if(length == 126)
        frame.length     = (_stream_.get() << 8) | _stream_.get();
    else {
        for(unsigned short i = 0; i < 8; i++) {
            frame.length <<= 8;
            frame.length |= _stream_.get();
        }
    }
    
    std::string data(frame.length, '\0');
    unsigned char maskKey[4];
    for(unsigned short i = 0; i < 4; i++) {
        if(frame.masked) maskKey[i] = (char)_stream_.get();
        else             maskKey[i] = '\0';
    }
    for(uint64_t i = 0; i < frame.length; i++)
        data[i] = (char)(_stream_.get()^maskKey[i%4]);
    frame.data = data.c_str();
    
    return frame;
}