/**
 * Created by TekuConcept on March 10, 2018
 */

#ifndef STRING_ENCODER_H
#define STRING_ENCODER_H

#include <string>

namespace Impact{
    namespace StringCodec {
        // RFC 3629: UTF-8, a transformation format of ISO 10646
        bool encodeUTF8(const std::string data, std::string& utf8);
        bool encodeUTF8(const char* data, unsigned int length, std::string& utf8);
        bool encodeUTF8(const unsigned int* data, unsigned int length, std::string& utf8);
        bool decodeUTF8(const std::string utf8, std::string data);
        bool decodeUTF8(const std::string utf8, char* data, unsigned int length);
        bool decodeUTF8(const std::string utf8, unsigned int* data, unsigned int length);
    }
}

#endif