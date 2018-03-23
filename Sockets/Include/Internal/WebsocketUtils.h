/**
 * Created by TekuConcept on March 8, 2018
 */

#ifndef _WEBSOCKET_Utils_H_
#define _WEBSOCKET_Utils_H_

#include <iostream>
#include <string>
#include <random>
#include "RFC/2616"

namespace Impact {
namespace Internal {
    typedef struct WSFrameContext {
        bool finished;
        unsigned char reserved;
        unsigned char opcode;
        bool masked;
        unsigned long long int length;
        unsigned char mask_key[4];
        WSFrameContext();
    } WSFrameContext;
    
    class WebsocketUtils {
        static std::string SECRET;
        static std::uniform_int_distribution<unsigned short> _distribution_;
        
        static std::string generateKey(std::mt19937);
        static std::string hashKey(std::string);
        static std::string getRequestKey(RFC2616::RequestMessage request);
        static std::string getResponseKey(RFC2616::ResponseMessage response);
        
        static uint16_t changeEndianness16(uint16_t val){ 
            return (val << 8) | ((val >> 8) & 0x00ff); 
        } 
        static uint64_t changeEndianness64(unsigned long long int val){ 
            return (val << 56) | 
                    ((val << 40) & 0x00ff000000000000) | 
                    ((val << 24) & 0x0000ff0000000000) | 
                    ((val << 8) & 0x000000ff00000000) | 
                    ((val >> 8) & 0x00000000ff000000) | 
                    ((val >> 24) & 0x0000000000ff0000) | 
                    ((val >> 40) & 0x000000000000ff00) | 
                    ((val >> 56) & 0x00000000000000ff); 
        } 
    public:
        static std::string SYN(std::iostream&, URI, std::mt19937);
        static std::string SYNACK(std::iostream&);
        static bool ACK(std::iostream&,std::string);
        
        static bool writeHeader(std::ostream&, WSFrameContext&, std::mt19937);
        static bool writeData(std::ostream&, WSFrameContext,
            const char*, unsigned int, int&);
        static bool readHeader(std::istream&, WSFrameContext&);
        static int readData(std::istream&,WSFrameContext,char*,int,int&);
        
        static void readHeader(const char[2],WSFrameContext&);
        static void readExtendedHeader(const char*,WSFrameContext&);
        static void xmaskData(char*,int,WSFrameContext,int&);
    };
}}

#endif