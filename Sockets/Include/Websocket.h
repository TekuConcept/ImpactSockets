/**
 * Created by TekuConcept on March 8, 2018
 */

#ifndef _WEBSOCKET_H_
#define _WEBSOCKET_H_

#include <iostream>
#include <streambuf>
#include <random>
#include <string>

#include "RFC/URI.h"
#include "RFC/6455"
#include "RFC/WebsocketUtils.h"

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
        // socket variables
        std::iostream& _stream_;
        URI _uri_;
        WS_TYPE _type_;
        std::mt19937 _engine_;
        RFC6455::STATE _connectionState_;
        
        // stream variables
        unsigned int _bufferSize_;
        char* _obuffer_;
    	char* _ibuffer_;
    	
    	// frame variables
    	Internal::WSFrameContext _inContext_;
    	Internal::WSFrameContext _outContext_;
    	bool _echo_;
        
        void pong(unsigned long long int length=0);
        void close(unsigned int code, std::string reason);
        
    public:
        Websocket(std::iostream& stream, URI uri, WS_TYPE type,
            unsigned int bufferSize=256);
        ~Websocket();
        bool shakeHands();

        void ping();
        void ping(std::string data);
        void push();
        int  push_s();
        void send();
        void close();
        
        // message finished
        // end of frame data
        
        WS_MODE in_mode();
        WS_MODE out_mode();
        void out_mode(WS_MODE mode);
        
        void wait();
        
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