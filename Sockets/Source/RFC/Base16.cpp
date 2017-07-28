/**
 * Created by TekuConcept on July 28, 2017
 */

#include "RFC/Base16.h"
#include <exception>

using namespace Impact;

const std::string Base16::ALPHABET = "0123456789ABCDEF";

Base16::Base16() {}

std::string Base16::encode(std::string data) {
    if(data.length() == 0) return "";
    
    const unsigned short SYMBOL = 0xF;
    const unsigned short SYMBOL_SIZE = 4;
    std::string result(data.length()*2, '0');
    for(unsigned int i = 0; i < data.length(); i++) {
        result[i*2  ] = ALPHABET[(int)(data[i] >> SYMBOL_SIZE)];
        result[i*2+1] = ALPHABET[(int)(data[i] & SYMBOL)];
    }
    
    return result;
}

std::string Base16::decode(std::string data) {
    bool success;
    std::string result = decode(data, success);
    if(!success) throw std::exception();
    return result;
}

std::string Base16::decode(std::string data, bool &status) {
    status = true;
    if(data.length() == 0) return "";
    
    std::string result(data.length()/2+data.length()%2, '\0');
    
    unsigned int idx = 0, jdx = 0;
    if(data.length()%2 != 0) {
        char c = (char)reverseLookup(data[idx]);
        if(c == '\x10') status = false;
        else result[jdx] = c;
        idx++;
        jdx++;
    }
    
    const unsigned short SYMBOL_SIZE = 4;
    for(; idx < data.length(); idx+=2) {
        if(!status) break;
        else {
            char c = (char)reverseLookup(data[idx]);
            char d = (char)reverseLookup(data[idx+1]);
            if(c == '\x10' || d == '\x10') status = false;
            else {
                result[jdx] = (char)((c << SYMBOL_SIZE) | d);
                jdx++;
            }
        }
    }
    
    return result;
}

unsigned char Base16::reverseLookup(const char c) {
    if     (c >= '0' && c <= '9') return (unsigned char)(c - '0');
    else if(c >= 'A' && c <= 'Z') return (unsigned char)(c - 'A' + 10);
    else if(c >= 'a' && c <= 'z') return (unsigned char)(c - 'a' + 10);
    else return (unsigned char)('\x10');
}