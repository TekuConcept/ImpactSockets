/**
 * Created by TekuConcept on July 25, 2017
 */

#ifndef _RFC_BASE64_H_
#define _RFC_BASE64_H_

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
