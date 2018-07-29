/**
 * Created by TekuConcept on July 28, 2018
 */

#ifndef _IMPACT_UTF8_H_
#define _IMPACT_UTF8_H_

#include <cstdint>
#include <string>

#define RFC3629 1

namespace impact {
    typedef uint16_t utf8;
    typedef std::basic_string<utf8> utf8string;
    
    // bool encode();
    // bool decode();
    
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
}

#endif