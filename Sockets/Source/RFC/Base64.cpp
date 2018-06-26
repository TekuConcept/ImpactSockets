/**
 * Created by TekuConcept on July 27, 2017
 */

#include "RFC/Base64.h"
#include <sstream>
#include <exception>

#include <iostream>
#include <iomanip>
#define DMSG(x) std::cerr << x << std::endl

using namespace Impact;

const std::string Base64::ALPHABET =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char Base64::PAD = '=';

Base64::Base64() {}

std::string Base64::encode(const std::string data) {
    if(data.length() == 0) return "";

    std::ostringstream os;
    const unsigned char SYMBOL      = 0x3F;
    const unsigned char SYMBOL_SIZE = 6;

    // encode
    unsigned int reg24;
    unsigned short reg16[4];
    for(unsigned int i = 0; (i+2) < data.length(); i+=3) {
        reg24 = ((0xFF&data[i    ]) << 16) |
                ((0xFF&data[i + 1]) <<  8) |
                ((0xFF&data[i + 2]));
        for(short j = 3; j >= 0; j--) {
            reg16[j] = reg24 & SYMBOL;
            reg24 >>= SYMBOL_SIZE;
        }
        for(unsigned char j = 0; j < 4; j++)
            os << ALPHABET[reg16[j]];
    }

    // determine padding
    unsigned char padCount = (data.length()%3 + (data.length()+1)%3) - 1;
    if(padCount == 0) return os.str();
    else {
        std::string padding(padCount, PAD);
        reg24 = ((0xFF&data[data.length() - 3 + padCount]) << 16);
        if(padCount == 1) reg24 |= ((0xFF&data[data.length() - 1]) <<  8);
        else reg24 >>= SYMBOL_SIZE;
        for(short i = (3 - padCount); i >= 0; i--) {
            reg24 >>= SYMBOL_SIZE;
            reg16[i] = reg24 & SYMBOL;
        }
        for(short i = 0; i <= (3 - padCount); i++)
            os << ALPHABET[reg16[i]];
        os << padding;
        return os.str();
    }
}

std::string Base64::decode(const std::string data) {
    bool success;
    auto result = decode(data, success);
    if(!success) throw std::exception();
    return result;
}

std::string Base64::decode(const std::string data, bool &status) {
    status = true;
    if(data.length() == 0) return "";

    std::ostringstream os;
    const unsigned char BYTE = 0xFF, SYMBOL_SIZE = 6;
    unsigned short padding = 0, tally = 0;
    unsigned int reg24 = 0;

    for(unsigned int i = 0; i < data.length(); i++) {
        if(data[i] == '=') {
            if((data.length() - i) <= 2) padding++;
            continue;
        }
        auto c = reverseLookup(data[i]);
        if(c == '\x40') {
            status = false;
            break;
        }
        else {
            reg24 |= c;
            tally++;
            if(tally == 4) {
                os << (unsigned char)(reg24 >> 16);
                os << (unsigned char)((reg24 >> 8)&BYTE);
                os << (unsigned char)(reg24&BYTE);
                tally = (unsigned short)0;
				reg24 = 0;
            }
            else reg24 <<= SYMBOL_SIZE;
        }
    }

    // take care of padded values
    if((padding + tally) == 4) {
        reg24 <<= (SYMBOL_SIZE * (padding - 1));
        os << (unsigned char)(reg24 >> 16);
        if(padding != 2) os << (unsigned char)((reg24 >> 8) & BYTE);
    }
    else if ((padding + tally) != 0) status = false;

    return os.str();
}

unsigned char Base64::reverseLookup(const char c) {
    if     (c >= 'A' && c <= 'Z') return (unsigned char)(c - 'A');
    else if(c >= 'a' && c <= 'z') return (unsigned char)(c - 'a' + 26);
    else if(c >= '0' && c <= '9') return (unsigned char)(c - '0' + 52);
    else if(c == '+') return (unsigned char)('\x3E');
    else if(c == '/') return (unsigned char)('\x3F');
    else return (unsigned char)('\x40');
}
