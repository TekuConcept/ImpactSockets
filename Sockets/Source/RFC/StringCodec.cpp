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

bool StringCodec::encodeUTF8(const char* data, unsigned int length,
    std::string& utf8) {
    std::ostringstream os;
    unsigned short symbol;
    for(unsigned int i = 0; i < length; i++) {
        symbol = static_cast<unsigned short>(data[i]&0xFF);
        if(symbol <= 0x007F) os << static_cast<unsigned char>(symbol);
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

bool StringCodec::decodeUTF8(const std::string utf8, std::string data) {
    return decodeUTF8(utf8,&data[0],data.length());
}

bool StringCodec::decodeUTF8(const std::string utf8, char* data,
    unsigned int length) {
    (void)utf8;
    (void)data;
    (void)length;
    return false;
}

bool StringCodec::decodeUTF8(const std::string utf8, unsigned int* data,
    unsigned int length) {
    (void)utf8;
    (void)data;
    (void)length;
    return false;
}