/**
 * Created by TekuConcept on July 26, 2017
 */

#ifndef RFC_STRING_H
#define RFC_STRING_H

#include <string>

namespace Impact{
namespace RFC2616 {
    struct custom_traits: std::char_traits<char> {
        static bool eq(char c, char d) {return std::tolower(c)==std::tolower(d);}
        static bool lt(char c, char d) {return std::tolower(c)<std::tolower(d);}
        static int compare(const char* p, const char* q, std::size_t n) {
            while (n--) {if (!eq(*p,*q)) return lt(*p,*q)?-1:1; p++; q++;}
            return 0;
        }
    };
    typedef std::basic_string<char,custom_traits> string;
    
    bool operator==(string lhs, std::string rhs);
    bool operator==(std::string lhs, string rhs);
    bool operator!=(string lhs, std::string rhs);
    bool operator!=(std::string lhs, string rhs);
}}

#endif