/**
 * Created by TekuConcept on March 8, 2018
 */

#ifndef _WEBSOCKET_H_
#define _WEBSOCKET_H_

#include <iostream>
#include <streambuf>
#include <random>
#include <string>
#include <memory>

#include "RFC/URI.h"
#include "RFC/6455"
#include "Internal/WebsocketUtils.h"

namespace Impact {
    enum class WS_TYPE {
        WS_CLIENT,
        WS_SERVER
    };
    
    enum class WS_MODE {
        TEXT,
        BINARY
    };
    
    class Websocket : private std::streambuf, public std::iostream {
        /////////////////////////////////////////
        // Class Variables                     //
        /////////////////////////////////////////
        
        std::iostream& _stream_;
        URI _uri_;
        WS_TYPE _type_;
        std::mt19937 _engine_;
        RFC6455::STATE _connectionState_;
        unsigned int _bufferSize_;
    	
        /////////////////////////////////////////
        // Out Stream                          //
        /////////////////////////////////////////
        
        char* _obuffer_;
    	int _outKeyOffset_;
    	bool _outContinued_;
    	unsigned char _outOpCode_;
    	
        /////////////////////////////////////////
        // In Stream                           //
        /////////////////////////////////////////
        
        char* _ibuffer_;
        int _inKeyOffset_;
        bool _inContinued_;
        Internal::WSFrameContext _inContext_;
        unsigned char _inOpCode_;
    	
        /////////////////////////////////////////
        // private functions                   //
        /////////////////////////////////////////
        
        int writeAndReset(bool finished, unsigned char opcode);
        void pong();
        void close(unsigned int code, std::string reason);
        
        int processNextFrame();
        unsigned long long int min(
            unsigned long long int, unsigned long long int);
        
    public:
        Websocket(std::iostream& stream, URI uri, WS_TYPE type,
            unsigned int bufferSize=256);
        ~Websocket();
        
        bool shakeHands();
        bool wait();

        void ping();
        void ping(std::string data);
        void push();
        int  push_s();
        void send();
        void close();
        
        WS_MODE in_mode();
        WS_MODE out_mode();
        void out_mode(WS_MODE mode);
        
        int sync();
        int overflow(int c);
        int underflow();
        
        friend std::ostream& text(std::ostream& stream);
        friend std::ostream& binary(std::ostream& stream);
        friend std::ostream& ping(std::ostream& stream);
        friend std::ostream& close(std::ostream& stream);
        friend std::ostream& send(std::ostream& stream);
        friend std::ostream& push(std::ostream& stream);
    };
}

#endif