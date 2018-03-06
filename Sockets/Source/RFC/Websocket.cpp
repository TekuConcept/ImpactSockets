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

DataFrame::DataFrame() : finished(true), reserved(0),
	opcode(0), masked(false), bad(false), data("") {}

Websocket::Websocket(std::iostream& stream, bool isClient)
    : _stream_(stream), _connectionState_(STATE::CLOSED),
	_distribution_(0,255), _isClient_(isClient) {
    auto now = std::chrono::high_resolution_clock::now();
    _engine_.seed(static_cast<unsigned int>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()
        ).count() + (size_t)this
        // 'this' prevents two instances having the same seed
    ));
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
    frame.opcode = OP_PONG;
    serializeOut(frame);
}

void Websocket::pong(DataFrame frame) {
    // RFC 6455 Section 5.5.3 Paragraph 3: Identical Application Data
    DataFrame nextFrame;
    initFrame(nextFrame);
    nextFrame.opcode = OP_PONG;
    if(frame.data.length() > 0)
        nextFrame.data = frame.data;
    serializeOut(nextFrame);
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
    frame.data       = text;
    serializeOut(frame);
}

void Websocket::sendBinary(const char* data, unsigned int length) {
    DataFrame frame;
    initFrame(frame);
    frame.opcode     = OP_BINARY;
    frame.data       = std::string(data, length);
    serializeOut(frame);
}

void Websocket::initFrame(DataFrame &frame) {
    frame.finished   = true;
    frame.reserved   = '\0';
    frame.masked     = _isClient_;
}

DataFrame Websocket::read() {
    DataFrame result = serializeIn();
    if(result.bad) return result;
    if(!(_isClient_ ^ result.masked)) {
        close();
    }
    else {
        switch(result.opcode) {
        case OP_PING:  pong(result); break;
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
    auto length = frame.data.length();
    if(length <= 125)
         l = (unsigned char)(length);
    else if (length <= 65535)
         l = (unsigned char)126;
    else l = (unsigned char)127;
    _stream_ << (unsigned char)((frame.masked << 7) | l);
    
    if(l == 126) {
        _stream_ << (char)(length >> 8);
        _stream_ << (char)(length & 0xFF);
    }
    else if (l == 127) {
        // TODO: MSb must be 0
        for(unsigned short i = 8; i <= 64; i+=8)
            _stream_ << (char)((length>>(64-i)) & 0xFF);
    }
    
    unsigned char maskKey[4];
    for(unsigned short i = 0; i < 4; i++) {
        if(frame.masked) {
            maskKey[i] = (unsigned char)_distribution_(_engine_);
            _stream_ << maskKey[i];
        }
        else maskKey[i] = '\0';
    }
    for(unsigned int i = 0; i < length; i++)
        _stream_ << (char)(frame.data[i]^maskKey[i%4]);
    
    _stream_ << std::flush;
}

DataFrame Websocket::serializeIn() {
    #define BAD_FRAME_TEST(c) if(!(_stream_ >> c)) return frame;
    DataFrame frame;
    frame.bad = true;
    char byte;
    
    BAD_FRAME_TEST(byte)
    frame.finished = ((byte & 0x80) == 0x80);
    frame.reserved = ((byte >> 4) & 0x7);
    frame.opcode   = (byte & 0xF);
    
    BAD_FRAME_TEST(byte)
    frame.masked         = ((byte & 0x80) == 0x80);
    unsigned char length = (byte & 0x7F);
    if(length <= 125)
        frame.data.resize(length, '\0');
    else if(length == 126) {
        char msb, lsb;
        BAD_FRAME_TEST(msb)
        BAD_FRAME_TEST(lsb)
        frame.data.resize((msb << 8) | lsb, '\0');
    }
    else {
		// 64-bit messages not supported yet
        unsigned long long int pad = 0;
        for(unsigned short i = 0; i < 8; i++) {
            BAD_FRAME_TEST(byte);
            pad = (pad << 8) | byte;
        }
        frame.data.resize((unsigned int)pad, '\0');
    }
    
    unsigned char maskKey[4];
    for(unsigned short i = 0; i < 4; i++) {
        if(frame.masked) {
            BAD_FRAME_TEST(byte);
            maskKey[i] = byte;
        }
        else maskKey[i] = '\0';
    }
	for (uint64_t i = 0; i < frame.data.length(); i++) {
		// limited array size; for 64-bit sizes, try queued processing
		BAD_FRAME_TEST(byte);
		frame.data[(const unsigned int)i] = (char)((int)(0xFF&byte) ^ maskKey[i%4]);
	}
    
    frame.bad = false;
    return frame;
    #undef BAD_FRAME_TEST
}