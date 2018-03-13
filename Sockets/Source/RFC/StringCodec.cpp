/**
 * Created by TekuConcept on March 10, 2018
 */

#include "RFC/StringCodec.h"
#include <sstream>

#include <iostream>
#include <iomanip>

using namespace Impact;
#define VERBOSE(x) std::cout << x << std::endl

/*
    RFC3629 Section 3. UTF-8 definition
    Char. number range  |        UTF-8 octet sequence
       (hexadecimal)    |              (binary)
    --------------------+---------------------------------------------
    0000 0000-0000 007F | 0xxxxxxx
    0000 0080-0000 07FF | 110xxxxx 10xxxxxx
    0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
    0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
*/

bool StringCodec::encodeUTF8(const std::string data, std::string& utf8) {
    return encodeUTF8(&data[0],data.length(),utf8);
}

#include <iomanip>
bool StringCodec::encodeUTF8(const char* data, unsigned int length,
    std::string& utf8) {
    std::ostringstream os;
    unsigned short symbol;
    for(unsigned int i = 0; i < length; i++) {
        symbol = static_cast<unsigned short>(data[i]&0xFF);
        if(symbol <= 0x007F)
            os << static_cast<unsigned char>(symbol);
        else {
            os << static_cast<unsigned char>(0xC0|(symbol>>6));
            os << static_cast<unsigned char>(0x80|(symbol&0x3F));
        }
    }
    utf8 = os.str();
    return true;
}

bool StringCodec::encodeUTF8(const unsigned int* data, unsigned int length,
    std::string& utf8) {
    std::ostringstream os;
    unsigned int symbol;
    for(unsigned int i = 0; i < length; i++) {
        symbol = data[i];
        if(symbol <= 0x00007F) os << static_cast<unsigned char>(symbol);
        else if(symbol <= 0x0007FF) {
            os << static_cast<unsigned char>(0xC0|(symbol>>6));
            os << static_cast<unsigned char>(0x80|(symbol&0x3F));
        }
        else if(symbol <= 0x00FFFF) {
            /* The definition of UTF-8 prohibits encoding character numbers
               between U+D800 and U+DFFF, which are reserved for use with the
               UTF-16 encoding form (as surrogate pairs) and do not directly
               represent characters. */
            if(symbol <= 0xDFFF && symbol >= 0xD800) return false;
            os << static_cast<unsigned char>(0xE0|(symbol>>12));
            os << static_cast<unsigned char>(0x80|((symbol>>6)&0x3F));
            os << static_cast<unsigned char>(0x80|(symbol&0x3F));
        }
        else if(symbol <= 0x10FFFF) {
            os << static_cast<unsigned char>(0xF0|(symbol>>18));
            os << static_cast<unsigned char>(0x80|((symbol>>12)&0x3F));
            os << static_cast<unsigned char>(0x80|((symbol>>6)&0x3F));
            os << static_cast<unsigned char>(0x80|(symbol&0x3F));
        }
        else return false;
    }
    utf8 = os.str();
    return true;
}

inline void __processUTF8Head(char u, unsigned int& s, int& status) {
    unsigned char n;
    if((u&0x80) == 0x00) n = u;
    else if((u&0xE0) == 0xC0) {
        if((n = u&0x1F) < 0x02)  status = UTF8_FAIL;
        else                     status = 1;
    }
    else if((u&0xF0) == 0xE0) {
        if((n = u&0x0F) == 0x00) status = 4;
        else                     status = 2;
    }
    else if((u&0xF8) == 0xF0) {
        if((n = u&0x07) > 0x04)  status = UTF8_FAIL;
        else if(n == 0x00)       status = 5;
        else                     status = 3;
    }
    else                         status = UTF8_FAIL;
    s = n;
}

inline void __processUTF8Tail(char u, unsigned int& s, int& status) {
    unsigned char n = u&0x3F;
    if(status == 4) {
        if(n < 0x20) {status = UTF8_FAIL;return;}
        else          status = 2;
    }
    else if(status == 5) {
        if(n < 0x10) {status = UTF8_FAIL;return;}
        else          status = 3;
    }
    
    s = (s<<6)|n;
    status--;
    
    // U+D800 through U+DFFF treated as invalid UTF8
    if(status == 0 && (s >= 0xD800 && s <= 0xDFFF)) status = UTF8_FAIL;
}

void StringCodec::decodeUTF8(char u, unsigned int& s, int& status) {
    if(status <= UTF8_FAIL) return;
    else if (status == UTF8_SUCCESS) {
        __processUTF8Head(u,s,status);
    }
    else if((u&0xC0) == 0x80) {
        __processUTF8Tail(u,s,status);
    }
    else status = UTF8_FAIL;
}
