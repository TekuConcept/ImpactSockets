/**
 * Created by TekuConcept on January 4, 2019
 */

#ifndef IMPACT_UTILS_STRING_OPS_H
#define IMPACT_UTILS_STRING_OPS_H

#include <string>
#include <vector>
#include <regex>

namespace impact {
namespace internal {
    void replace_all(std::string* value, std::string* select,
        std::string* change);
    
    void trim_whitespace(std::string* value);
    
    std::vector<std::string> split(const std::string* value,
        const std::string* delimiter);
    std::vector<std::string> split(const std::string* value,
        std::string delimiter);
    std::vector<std::string> split(const std::string* value,
        std::regex delimiter);
    std::vector<std::string> split(const std::string* value,
        char delimiter, char escape);
}}

#endif
