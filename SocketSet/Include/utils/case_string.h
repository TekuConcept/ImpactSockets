/**
 * Created by TekuConcept on December 17, 2018
 * Note: Case-insensitive string implementation
 */

#ifndef IMPACT_UTILS_CASE_STRING_H
#define IMPACT_UTILS_CASE_STRING_H

#include <string>
#include <iostream>

namespace impact {
    struct case_string_traits: std::char_traits<char> {
        static bool eq(char c, char d) {return std::tolower(c)==std::tolower(d);}
        static bool lt(char c, char d) {return std::tolower(c)<std::tolower(d);}
        static int compare(const char* p, const char* q, std::size_t n) {
            while (n--) { if (!eq(*p,*q)) return lt(*p,*q)?-1:1; p++; q++; }
            return 0;
        }
    };
    typedef std::basic_string<char,case_string_traits> case_string;

    bool operator==(const case_string& lhs, const std::string& rhs);
    bool operator==(const std::string& lhs, const case_string& rhs);
    bool operator!=(const case_string& lhs, const std::string& rhs);
    bool operator!=(const std::string& lhs, const case_string& rhs);
    
    std::ostream& operator<<(std::ostream& lhs, const case_string& rhs);
    std::istream& operator>>(std::istream& lhs, case_string& rhs);
    
    case_string&& move(std::string& t);
    std::string&& move(case_string& t);
    void swap(std::string& lhs, case_string& rhs);
    void swap(case_string& lhs, std::string& rhs);
}

#endif
