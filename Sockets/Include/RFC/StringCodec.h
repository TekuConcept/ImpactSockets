/**
 * Created by TekuConcept on March 10, 2018
 */

#ifndef STRING_ENCODER_H
#define STRING_ENCODER_H

#include <string>

namespace Impact{
    namespace StringCodec {
        // RFC 3629: UTF-8, a transformation format of ISO 10646
        #define UTF8_BOM      0xFEFF /* "ZERO WIDTH NO-BREAK SPACE" */
        #define UTF8_FAIL    -1
        #define UTF8_SUCCESS  0
        
        bool encodeUTF8(const std::string data, std::string& utf8);
        bool encodeUTF8(const char* data, unsigned int length, std::string& utf8);
        bool encodeUTF8(const unsigned int* data, unsigned int length,
            std::string& utf8);
        
        void decodeUTF8(char utf8byte, unsigned int& codepoint, int& status);
        
        unsigned long long int encodeLength(const char* data,
            unsigned int length);
    }
}

#endif