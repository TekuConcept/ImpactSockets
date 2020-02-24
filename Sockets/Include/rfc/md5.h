/**
 * Created by TekuConcept on January 30, 2019
 * Endian-Agnostic, type-size-independent MD5 implementation
 */

#ifndef IMPACT_RFC_MD5_H
#define IMPACT_RFC_MD5_H

#include <cstdint>
#include <string>
#include "utils/environment.h"

#define RFC1321 1

namespace impact {
    class md5 {
    public:
        md5() = delete;
        
        static std::string digest(std::string message);
        
    private:
        #if defined(HAVE_UINT32_T)
            typedef uint32_t var;
        #else
            typedef unsigned long int var;
        #endif
        
        static var s[64];
        static var K[64];
        
        static void _S_preprocess(std::string*);
        static std::string _S_process(const std::string&);
    };
}

#endif
