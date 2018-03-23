/**
 * Created by TekuConcept on March 8, 2018
 */

#include "Websocket.h"
#include "RFC/StringCodec.h"

#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>
#include <climits>
#include <stdexcept>

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

#define VERBOSE(x) std::cout << x << std::endl
#define FRAME_HEADER_SIZE 2
#define CTOR_INIT(c, s, u, t, b)\
    std::iostream(this),\
    _socket_(s), _context_(c), _uri_(u), _type_(type),\
    _connectionState_(STATE::CLOSED), _bufferSize_(b),\
    _obuffer_(new char[b+1]), _outKeyOffset_(0), _outContinued_(false),\
    _outOpCode_(OP_TEXT), _ibuffer_(new char[b+1]), _inKeyOffset_(0),\
    _inContinued_(false), _inOpCode_(OP_TEXT), _readState_(0)

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
|                                                                             |
|  WEBSOCKET CLASS FUNCTIONS & UTILITIES                                      |
|                                                                             |
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Websocket::Websocket(IOContext& context, std::shared_ptr<TcpClient> socket,
    URI uri, WS_TYPE type, unsigned int bufferSize) :
    CTOR_INIT(context, socket, uri, type, bufferSize) {
    if(socket == nullptr)
        throw std::runtime_error("Received Null Socket Pointer");
    init();
}


void Websocket::init() {
    auto now = std::chrono::high_resolution_clock::now();
    _engine_.seed(static_cast<unsigned int>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()
        ).count() + (size_t)this
        // 'this' prevents two instances having the same seed
    ));
    
    setp(_obuffer_, _obuffer_ + _bufferSize_ - 1);
	setg(_ibuffer_, _ibuffer_ + _bufferSize_ - 1, _ibuffer_ + _bufferSize_ - 1);
}


unsigned long long int Websocket::min(
    unsigned long long int A, unsigned long long int B) {
    if(A < B) return A;
    else return B;
}


Websocket::~Websocket() {
    if(_reading_.valid()) _reading_.wait();
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
        key = WebsocketUtils::SYN(*_socket_,_uri_,_engine_);
        success = WebsocketUtils::ACK(*_socket_,key);
        break;
    case WS_TYPE::WS_SERVER:
        key = WebsocketUtils::SYNACK(*_socket_);
        success = (key.length() > 0);
        break;
    }
    
    _connectionState_ = success?STATE::OPEN:STATE::CLOSED;
    if(success) enqueue();
    return success;
}


void Websocket::enqueue() {
    _readState_ = 0;
    _reading_ = _context_.enqueue(_socket_->getSocket(),_iswap_,
        FRAME_HEADER_SIZE,[&](char*& b, int& l){whenReadDone(b,l);});
}


WS_MODE Websocket::in_mode() {
    if(_inContext_.opcode == OP_TEXT)
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


RFC6455::STATE Websocket::getConnectionState() {
    return _connectionState_;
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
|                                                                             |
|  SEND FUNCTIONS                                                             |
|                                                                             |
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


// enqueue ping, do not write during stream
void Websocket::ping() {
    if(_connectionState_ == STATE::CLOSED) return;
    WSFrameContext context;
    context.opcode = OP_PING;
    context.masked = _type_==WS_TYPE::WS_CLIENT;
    std::lock_guard<std::mutex> lock(_socmtx_);
    WebsocketUtils::writeHeader(*_socket_, context, _engine_);
}


// enqueue ping, do not write during stream
void Websocket::ping(std::string data) {
    if(_connectionState_ == STATE::CLOSED) return;
    WSFrameContext context;
    context.opcode = OP_PING;
    context.masked = _type_==WS_TYPE::WS_CLIENT;
    context.length = data.length();
    auto keyOffset = 0;
    std::lock_guard<std::mutex> lock(_socmtx_);
    WebsocketUtils::writeHeader(*_socket_, context, _engine_);
    WebsocketUtils::writeData(*_socket_, context, &data[0],
        data.length(), keyOffset);
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
    context.length = int(pptr() - pbase());
    if(_outOpCode_ == OP_TEXT)
        context.length = StringCodec::encodeLength(pbase(),context.length);
    
    std::lock_guard<std::mutex> lock(_socmtx_);
    if(WebsocketUtils::writeHeader(*_socket_, context, _engine_)) {
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
                
                WebsocketUtils::writeData(*_socket_, context,
                    &utf8[0], utf8.length(), _outKeyOffset_);
            }
        }
        else WebsocketUtils::writeData(*_socket_, context, pbase(),
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
    _readState_ = 3;
    WSFrameContext context;
    context.finished = true;
    context.opcode = OP_CLOSE;
    context.masked = _type_ == WS_TYPE::WS_CLIENT;
    
    bool shouldWriteReason = !(
        code == CODE_NO_STATUS ||
        code == CODE_ABNORMAL ||
        code == CODE_TLS_HANDSHAKE);
    if(shouldWriteReason) context.length = reason.length() + 2;
    
    std::lock_guard<std::mutex> lock(_socmtx_);
    WebsocketUtils::writeHeader(*_socket_,context,_engine_);
    
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
            *_socket_,context,(const char*)&_code_,2,keyOffset);
        WebsocketUtils::writeData(
            *_socket_,context,&reason[0],reason.length(),keyOffset);
    }

    _connectionState_ = STATE::CLOSED;
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
|                                                                             |
|  RECEIVE FUNCTIONS                                                          |
|                                                                             |
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


void Websocket::whenReadDone(char*& nextBuffer, int& nextLength) {
    switch(+_readState_) {
    case 0: state2ByteHeader(nextBuffer, nextLength); break;
    case 1: stateExtendedHeader(nextBuffer, nextLength); break;
    case 2: stateBody(nextBuffer, nextLength); break;
    default: // close
        nextBuffer = NULL;
        nextLength = 0;
        break;
    }
}


void Websocket::state2ByteHeader(char*& nextBuffer, int& nextLength) {
    WebsocketUtils::readHeader(_iswap_, _inContext_);

    if(_inContext_.length == 126) nextLength = 2;
    else if(_inContext_.length == 127) nextLength = 8;
    else nextLength = 0;
    nextLength += _inContext_.masked?4:0;

    if((!_inContext_.masked) ^ (this->_type_ == WS_TYPE::WS_CLIENT)) {
        nextBuffer = NULL;
        nextLength = 0;
        close(CODE_PROTO_ERR,"Bad Frame Mask");
        return;
    }

    if(nextLength > 0) {
        nextBuffer = _iswap_;
        _readState_ = 1;
    }
    else {
        processFrame();
        stateBodyHelper(nextBuffer,nextLength);
    }
}


void Websocket::stateExtendedHeader(char*& nextBuffer, int& nextLength) {
    WebsocketUtils::readExtendedHeader(_iswap_, _inContext_);
    processFrame();
    stateBodyHelper(nextBuffer,nextLength);
}


void Websocket::stateBodyHelper(char*& nextBuffer, int& nextLength) {
    if(_inContext_.length > 0) {
        switch(_inContext_.opcode) {
        case OP_PING:
        case OP_PONG:
        case OP_CLOSE:
            nextBuffer = _iswap_;
            nextLength = min(128,_inContext_.length);
            break;
        default:
            nextBuffer = eback();
            nextLength = min(_bufferSize_,_inContext_.length);
            break;
        }
        _inContext_.length -= nextLength;
        _echoContext_.length = nextLength;
        _readState_ = 2;
        _inKeyOffset_ = 0;
    }
    else {
        nextBuffer = _iswap_;
        nextLength = FRAME_HEADER_SIZE;
        _readState_ = 0;
    }
}


void Websocket::stateBody(char*& nextBuffer, int& nextLength) {
    // re-enque on underflow
    // if context.opcode == text: decodeUTF8
    // setg(eback(), eback(), eback() + len);
    if(_inContext_.opcode == OP_PING) {
        auto temp = _inKeyOffset_;
        WebsocketUtils::xmaskData(_iswap_,_echoContext_.length,_inContext_,
            _inKeyOffset_);
        _inKeyOffset_ = temp;
        
        std::cout << "-> [" << _echoContext_.length << "] ";
        for(int i = 0; i < static_cast<int>(_echoContext_.length); i++) {
            std::cout << _iswap_[i];
        }
        std::cout << std::endl;
        
        WebsocketUtils::writeData(*_socket_,_echoContext_,_iswap_,
            _echoContext_.length,_inKeyOffset_);
    }
    else if(_inContext_.opcode == OP_PONG) {
        std::cout << "-> [" << _echoContext_.length << "] ";
        for(int i = 0; i < static_cast<int>(_echoContext_.length); i++) {
            std::cout << _iswap_[i];
        }
        std::cout << std::endl;
    }
    
    if(_inContext_.opcode == OP_TEXT || _inContext_.opcode == OP_BINARY ||
        _inContext_.opcode == OP_CONTINUE) {
        nextBuffer = pbase(); // NULL
        nextLength = min(_inContext_.length,_bufferSize_); // 0
        _inContext_.length -= nextLength;
        _echoContext_.length = nextLength;
        // decodeUTF8 if TEXT
    }
    else {
        nextBuffer = _iswap_;
        if(_inContext_.length > 0) {
            nextLength = min(128,_inContext_.length);
            _inContext_.length -= nextLength;
            _echoContext_.length = nextLength;
        }
        else {
            nextLength = 2;
            _readState_ = 0;
        }
    }
}


void Websocket::processFrame() {
    switch(_inContext_.opcode) {
    case OP_BINARY:
    case OP_TEXT:
    case OP_CONTINUE:
        VERBOSE("-> data");
        break;
    case OP_CLOSE:
        VERBOSE("-> close");
        if(_connectionState_ != STATE::CLOSED) close();
        break;
    case OP_PING:
        VERBOSE("-> ping");
        _echoContext_.opcode = OP_PONG;
        _echoContext_.masked = _type_==WS_TYPE::WS_CLIENT;
        _echoContext_.length = _inContext_.length;
        {
            std::lock_guard<std::mutex> lock(_socmtx_);
            WebsocketUtils::writeHeader(*_socket_, _echoContext_, _engine_);
        }
        break;
    case OP_PONG:
        VERBOSE("-> pong");
        /* flush input stream */
        break;
    default:
        if(_connectionState_ != STATE::CLOSED)
            close(CODE_PROTO_ERR, "Unknown OpCode");
        break;
    }
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
|                                                                             |
|  IOSTREAM FUNCTIONS                                                         |
|                                                                             |
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


int Websocket::sync() {
    std::cout << "-> sync" << std::endl;
    return push_s();
}


int Websocket::overflow(int c) {
    std::cout << "-> overflow" << std::endl;
    if(push_s() < 0) return EOF;
    else if(c != EOF) put((char)c);
    return c;
}


int Websocket::underflow() {
    std::cout << "-> underflow" << std::endl;
    // RFC6455 Section 7.1.7 Paragraph 2
    if(_connectionState_ == STATE::CLOSED) return EOF;
    
    if(_reading_.valid()) _reading_.wait();
    else return EOF;
    
    // auto result = _reading_.get();
    // if(result <= 0) { close(); state = 3; return EOF }
    // else enqueue(state?more data:next frame)
    
    // if(!wait()) return EOF;
    // auto len = min(_inContext_.length,_bufferSize_);
    // WebsocketUtils::readData(
    //     *_socket_, _inContext_, eback(), len, _inKeyOffset_);
    // setg(eback(), eback(), eback() + len);
    // _inContext_.length -= len;
    
    return *eback();
}

int Websocket::uflow() {
  if ( underflow() == EOF ) return EOF;
  gbump(1);
  return gptr()[-1];
}