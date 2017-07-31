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

namespace Impact {
namespace RFC6455 {
    typedef struct DataFrame {
        bool finished;
        unsigned char reserved;
        unsigned char opcode;
        bool masked;
        uint64_t length;
        const char* data;
    } DataFrame;
    
    class Websocket {
    public:
        Websocket(std::iostream& stream, bool isClient);
        virtual ~Websocket();
        
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
        std::uniform_int_distribution<uint8_t> _distribution_;
        
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