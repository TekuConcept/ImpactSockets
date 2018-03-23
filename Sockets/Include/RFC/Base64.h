/**
 * Created by TekuConcept on July 25, 2017
 */

#ifndef RFC_BASE64_H
#define RFC_BASE64_H

#include <string>

namespace Impact {
    class Base64 {
    public:
        static std::string encode(const std::string data);
        static std::string decode(const std::string data);
        static std::string decode(const std::string data, bool &status);
    
    private:
        static const std::string ALPHABET;
        static const char PAD;
        
        Base64();
        static unsigned char reverseLookup(const char c);
    };
}

#endif