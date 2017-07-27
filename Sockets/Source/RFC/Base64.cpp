/**
 * Created by TekuConcept on July 27, 2017
 */

#include "RFC/Base64.h"
#include <sstream>

#include <iostream>
#define DMSG(x) std::cerr << x << std::endl

using namespace Impact;
using namespace RFC4648;

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
        reg24 = (data[i] << 16) | (data[i + 1] << 8) | (data[i + 2]);
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
        reg24 = (data[data.length() - 3 + padCount] << 16);
        if(padCount == 1) reg24 |= (data[data.length() - 1] <<  8);
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

bool Base64::validate(const char c) {
    return ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            (c == '+' || c == '/'));
}