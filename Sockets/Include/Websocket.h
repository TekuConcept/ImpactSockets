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
#include <mutex>
#include <future>

#include "RFC/URI.h"
#include "RFC/6455"
#include "Internal/WebsocketUtils.h"
#include "IOContext.h"
#include "TcpClient.h"

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
        
        std::shared_ptr<TcpClient> _socket_;
        std::mutex _socmtx_;
        IOContext& _context_;
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
        unsigned char _inOpCode_;
        Internal::WSFrameContext _inContext_, _echoContext_;
        char _iswap_[128];
        std::atomic<int> _readState_;
        std::future<int> _reading_;
    	
        /////////////////////////////////////////
        // private functions                   //
        /////////////////////////////////////////
        
        void close(unsigned int code, std::string reason);
        void init();
        unsigned long long int min(
            unsigned long long int, unsigned long long int);
        void enqueue();
        
        int writeAndReset(bool finished, unsigned char opcode);
        
        void whenReadDone(char*&,int&);
        void state2ByteHeader(char*&,int&);
        void stateExtendedHeader(char*&,int&);
        void stateBodyHelper(char*&,int&);
        void stateBody(char*&,int&);
        void processFrame();
        
    public:
        Websocket(IOContext& context, std::shared_ptr<TcpClient> socket,
            URI uri, WS_TYPE type, unsigned int bufferSize=256);
        ~Websocket();
        
        bool shakeHands();
        WS_MODE in_mode();
        WS_MODE out_mode();
        void out_mode(WS_MODE mode);

        void ping();
        void ping(std::string data);
        void push();
        int  push_s();
        void send();
        void close();
        
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