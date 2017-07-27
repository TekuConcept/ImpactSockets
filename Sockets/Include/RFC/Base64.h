/**
 * Created by TekuConcept on July 25, 2017
 */

#ifndef RFC_BASE64_H
#define RFC_BASE64_H

#include <string>

namespace Impact {
namespace RFC4648 {
    class Base64 {
    public:
        static std::string encode(const std::string data);
    
    private:
        static const std::string ALPHABET;
        static const char PAD;
        
        Base64();
        bool validate(const char c);
    };
}}

#endif