/**
 * Created by TekuConcept on July 28, 2017
 */

#ifndef RFC_BASE16_H
#define RFC_BASE16_H

#include <string>

namespace Impact {
    class Base16{
    public:
        static std::string encode(std::string data);
        static std::string decode(std::string data);
        static std::string decode(std::string data, bool &status);
    
    private:
        static const std::string ALPHABET;
        
        Base16();
        static unsigned char reverseLookup(const char c);
    };
}

#endif