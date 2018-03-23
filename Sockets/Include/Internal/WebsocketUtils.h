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
        
        // todo: update with system endian consideration
        static unsigned short changeEndianness16(unsigned short val);
        static unsigned long long int changeEndianness64(
            unsigned long long int val);
        
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