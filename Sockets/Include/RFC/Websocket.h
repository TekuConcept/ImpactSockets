/**
 * Created by TekuConcept on July 28, 2017
 */

#ifndef RFC_WEBSOCKET_H
#define RFC_WEBSOCKET_H

#include <string>
#include <random>
#include <iostream>
#include <stdint.h>
#include "RFC/Const6455.h"

#define WS_OP_CONTINUE 0
#define WS_OP_TEXT     1
#define WS_OP_BINARY   2
#define WS_OP_CLOSE    8
#define WS_OP_PING     9
#define WS_OP_PONG    10

namespace Impact {
namespace RFC6455 {
    typedef struct DataFrame {
        bool finished;
        unsigned char reserved;
        unsigned char opcode;
        bool masked;
        bool bad;
        std::string data;
		DataFrame();
    } DataFrame;
    
    class RFCWebsocket {
    public:
        RFCWebsocket(std::iostream& stream, bool isClient);
        virtual ~RFCWebsocket();
        
        virtual bool initiateHandshake();
        
        void ping();
        void pong();
        void close();
        
        void sendText(std::string text);
        void sendBinary(const char* data, unsigned int length);
        
        DataFrame read();
        STATE getState();
        
    protected:
        std::iostream& _stream_;
        std::string _key_;
        STATE _connectionState_;
        
        std::mt19937 _engine_;
        std::uniform_int_distribution<unsigned short> _distribution_;
        
        const std::string SECRET = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    
    private:
        bool _isClient_;
        
        void pong(DataFrame frame);
        void initFrame(DataFrame &frame);
        void serializeOut(DataFrame frame);
        DataFrame serializeIn();
    };
}}

#endif