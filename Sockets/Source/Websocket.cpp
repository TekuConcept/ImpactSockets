/**
 * Created by TekuConcept on March 8, 2018
 */

#include "Websocket.h"
#include "RFC/StringCodec.h"

#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>

using namespace Impact;
using namespace RFC6455;
using namespace Internal;

#define OP_CONTINUE 0
#define OP_TEXT     1
#define OP_BINARY   2
#define OP_CLOSE    8
#define OP_PING     9
#define OP_PONG     10

#define CODE_NORMAL           1000 /* normal closure              */
#define CODE_GOING_AWAY       1001 /* going away                  */
#define CODE_PROTO_ERR        1002 /* protocol error              */
#define CODE_UNSUPPORTED      1003 /* unsupported data            */
#define CODE_RESERVED         1004 /* reserved code               */
#define CODE_NO_STATUS        1005 /* no status received          */
#define CODE_ABNORMAL         1006 /* abnormal closure            */
#define CODE_INVALID          1007 /* invalide frame payload data */
#define CODE_POLICY_VIOLATION 1008 /* policy violation            */
#define CODE_MSG_TOO_BIG      1009 /* message too big             */
#define CODE_MANDATORY_EXT    1010 /* mandatory extension         */
#define CODE_INTERNAL_ERROR   1011 /* internal server error       */
#define CODE_TLS_HANDSHAKE    1015 /* TLS handshake               */


Websocket::Websocket(
    std::iostream& stream, URI uri, WS_TYPE type, unsigned int bufferSize) :
    std::iostream(this), _stream_(stream), _uri_(uri), _type_(type),
    _connectionState_(STATE::CLOSED), _bufferSize_(bufferSize),
    _obuffer_(new char[bufferSize+1]), _outKeyOffset_(0), _outContinued_(false),
    _outOpCode_(OP_TEXT), _ibuffer_(new char[bufferSize+1]),
    _inKeyOffset_(0),  _inContinued_(false), _inOpCode_(OP_TEXT)
    {
    
    auto now = std::chrono::high_resolution_clock::now();
    _engine_.seed(static_cast<unsigned int>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()
        ).count() + (size_t)this
        // 'this' prevents two instances having the same seed
    ));
    
    setp(_obuffer_, _obuffer_ + bufferSize - 1);
	setg(_ibuffer_, _ibuffer_ + bufferSize - 1, _ibuffer_ + bufferSize - 1);
}


Websocket::~Websocket() {
    delete[] _obuffer_;
    delete[] _ibuffer_;
}


bool Websocket::shakeHands() {
    if(_connectionState_ == STATE::OPEN) return false;
    _connectionState_ = STATE::CONNECTING;
    
    std::string key;
    bool success = false;
    switch(_type_) {
    case WS_TYPE::WS_CLIENT:
        key = WebsocketUtils::SYN(_stream_,_uri_,_engine_);
        success = WebsocketUtils::ACK(_stream_,key);
        break;
    case WS_TYPE::WS_SERVER:
        key = WebsocketUtils::SYNACK(_stream_);
        success = (key.length() > 0);
        break;
    }
    
    _connectionState_ = success?STATE::OPEN:STATE::CLOSED;
    return success;
}


// enqueue ping, do not write during stream
void Websocket::ping() {
    if(_connectionState_ == STATE::CLOSED) return;
    WSFrameContext context;
    context.opcode = OP_PING;
    context.masked = _type_==WS_TYPE::WS_CLIENT;
    WebsocketUtils::writeHeader(_stream_, context, _engine_);
}


// enqueue ping, do not write during stream
void Websocket::ping(std::string data) {
    if(_connectionState_ == STATE::CLOSED) return;
    WSFrameContext context;
    context.opcode = OP_PING;
    context.masked = _type_==WS_TYPE::WS_CLIENT;
    context.length = data.length();
    auto keyOffset = 0;
    WebsocketUtils::writeHeader(_stream_, context, _engine_);
    WebsocketUtils::writeData(_stream_, context, &data[0],
        data.length(), keyOffset);
}


void Websocket::pong(unsigned long long int) {
    // if(_connectionState_ == STATE::CLOSED) return;
    // WSFrameContext context;
    // context.opcode = OP_PONG;
    // context.masked = _type_==WS_TYPE::WS_CLIENT;
    // context.length = length;
    // WebsocketUtils::writeHeader(_stream_, context, _engine_);
    
    // // RFC 6455 Section 5.5.3 Paragraph 3: Identical Application Data
    // if(_echo_) {
    //     _outContext_.masked = context.masked;
    //     for(auto i = 0; i < 4; i++)
    //         _outContext_.mask_key[i] = context.mask_key[i];
    // }
}


void Websocket::push() {
    push_s();
}


int Websocket::push_s() {
    if(_connectionState_ == STATE::CLOSED) return -1;
    bool finished = false;
    unsigned char opcode;
    if(!_outContinued_) {
        _outContinued_ = true;
        opcode = _outOpCode_;
    }
    else opcode = OP_CONTINUE;
    
    return writeAndReset(finished, opcode);
}


void Websocket::send() {
    if(_connectionState_ == STATE::CLOSED) return;
    bool finished = true;
    unsigned char opcode;
    if(_outContinued_) {
        opcode = OP_CONTINUE;
        _outContinued_ = false;
    }
    else opcode = _outOpCode_;
    
    writeAndReset(finished, opcode);
}


// don't write header in stream mode
// check queue if done streaming
int Websocket::writeAndReset(bool finished, unsigned char opcode) {
    WSFrameContext context;
    context.finished = finished;
    context.opcode   = opcode;
    context.masked   = _type_ == WS_TYPE::WS_CLIENT;
    context.length   = int(pptr() - pbase());
    
    if(WebsocketUtils::writeHeader(_stream_, context, _engine_)) {
        _outKeyOffset_ = 0;
        if(_outOpCode_ == OP_TEXT) {
            // Avoid worst case utf8 stream length:
            // 2*context.length > UINT32_MAX by writing the
            // first and second half of the data seperately
            std::string utf8;
            unsigned int length[2];
            
            length[0] = static_cast<unsigned int>(context.length>>2);
            length[1] = static_cast<unsigned int>(context.length-length[0]);
            int offset = 0;
            
            for(auto i = 0; i < 2; i++) {
                StringCodec::encodeUTF8(pbase()+offset,length[i],utf8);
                offset = length[0];
                
                WebsocketUtils::writeData(_stream_, context,
                    &utf8[0], utf8.length(), _outKeyOffset_);
            }
        }
        else WebsocketUtils::writeData(_stream_, context, pbase(),
            static_cast<unsigned int>(context.length), _outKeyOffset_);
        
        setp(pbase(), epptr());
        return 0;
    }
    return -1;
}


// if streaming, send 0s until streaming finished
// then send close frame
void Websocket::close() {
    close(CODE_NORMAL,"");
}


void Websocket::close(unsigned int code, std::string reason) {
    if(_connectionState_ == STATE::CLOSED) return;
    WSFrameContext context;
    context.finished = true;
    context.opcode = OP_CLOSE;
    context.masked = _type_ == WS_TYPE::WS_CLIENT;
    
    bool shouldWriteReason = !(
        code == CODE_NO_STATUS ||
        code == CODE_ABNORMAL ||
        code == CODE_TLS_HANDSHAKE);
    if(shouldWriteReason) context.length = reason.length() + 2;
    
    WebsocketUtils::writeHeader(_stream_,context,_engine_);
    
    if(shouldWriteReason) {
        /**
         * RFC6455 Section 5.5.1 Paragraph 2
         * 
         * If there is a body, the first two bytes of
         * the body MUST be a 2-byte unsigned integer (in network byte order)
         * representing a status code with value /code/ defined in Section 7.4.
         */
        unsigned short _code_ = code&0xFFFF;
        unsigned int endian = 1;
        if (((char*)&endian)[0]) {
            // little endian, swap bytes
            _code_ = ((_code_&0xFF)<<8) | (_code_>>8);
        }
        auto keyOffset = 0;
        WebsocketUtils::writeData(
            _stream_,context,(const char*)&_code_,2,keyOffset);
        WebsocketUtils::writeData(
            _stream_,context,&reason[0],reason.length(),keyOffset);
    }

    _connectionState_ = STATE::CLOSED;
}


WS_MODE Websocket::in_mode() {
    if(_inOpCode_ == OP_TEXT)
        return WS_MODE::TEXT;
    return WS_MODE::BINARY;
}


WS_MODE Websocket::out_mode() {
    if(_outOpCode_ == OP_TEXT)
        return WS_MODE::TEXT;
    return WS_MODE::BINARY;
}

// prevent change in streaming mode
void Websocket::out_mode(WS_MODE mode) {
    switch(mode) {
    case WS_MODE::TEXT:   _outOpCode_ = OP_TEXT;   break;
    case WS_MODE::BINARY: _outOpCode_ = OP_BINARY; break;
    }
}


void Websocket::wait() {
    if(_connectionState_ == STATE::CLOSED) return;
    // if(_inContext_._processed_ != _inContext_.length) return;
    bool available = false;
    do {
        WSFrameContext context;
        if(!WebsocketUtils::readHeader(_stream_,context)) {
            close(CODE_GOING_AWAY, "Connection Timed Out");
            return;
        }
        // The server MUST close the connection upon receiving a
        // frame that is not masked. A server MUST NOT mask any
        // frames that it sends to the client.
        if(_type_ == WS_TYPE::WS_SERVER && !context.masked) {
            close(CODE_PROTO_ERR, "Received Unmasked Frame");
            return;
        }
        
    //     switch(context.opcode) {
    //     case OP_CLOSE:
    //         // MAY process application data in a future update
    //         close();
    //         return;
    //     case OP_PING:
    //         available = context.length > 0;
    //         if(available) {
    //             _echo_ = true;
    //             _inContext_.finished = true;
    //             _inContext_.length   = context.length;
    //             _inContext_.masked   = context.masked;
    //             for(auto i = 0; i < 4; i++)
    //                 _inContext_.mask_key[i] = context.mask_key[i];
    //             _inContext_._processed_ = 0;
    //         }
    //         pong(context.length);
    //         break;
    //     case OP_PONG:
    //         // read in and discard all data
    //         // warning: infinite loop on disconnect
    //         break;
    //     default:
    //         /*
    // 		case data:
    // 			set input context
    // 			data = true
    // 			break
    //         */
    //         break;
    //     }
    } while(!available);
}


int Websocket::sync() {
    return push_s();
}


int Websocket::overflow(int c) {
    if(push_s() < 0) return EOF;
    else if(c != EOF) put((char)c);
    return c;
}


int Websocket::underflow() {
    // RFC6455 Section 7.1.7 Paragraph 2
    if(_connectionState_ == STATE::CLOSED) return EOF;
    
    // - return when either the stream buffer
    //   is filled or the entire frame was read in
    // - return EOF on close frame
    return EOF;
}