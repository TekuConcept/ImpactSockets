/**
 * Created by TekuConcept on January 4, 2018
 */

#include <sstream>
#include "utils/string_ops.h"

using namespace impact;


void
internal::replace_all(
    std::string* __value,
    std::string* __select,
    std::string* __change)
{
    if (__value == NULL || __select == NULL) return;
    if (__select->empty()) return;

    std::string change = (__change == NULL) ? "" : *__change;
    std::ostringstream os;

    size_t first = 0, last;
    while ((last = __value->find(*__select, first)) !=
        std::string::npos) {
        os << __value->substr(first, last - first) << change;
        first = last + __select->size();
    }
    os << __value->substr(first);

    *__value = os.str();
}


void
internal::trim_whitespace(std::string* __value)
{
    if (__value == NULL) return;
    const std::string k_whitespace = " \t";
    size_t begin = __value->find_first_not_of(k_whitespace);
    if (begin == std::string::npos) return;
    size_t end = __value->find_last_not_of(k_whitespace);
    size_t length = end - begin + 1;
    if (length == __value->size()) return;
    *__value = __value->substr(begin, length);
}


std::vector<std::string>
internal::split(
    const std::string* __value,
    const std::string* __delimiter)
{
    if (__value == NULL) return {};
    else if (__delimiter == NULL) return { *__value };
    
    size_t first = 0, last;
    std::vector<std::string> tokens;
    while ((last = __value->find(*__delimiter, first)) !=
        std::string::npos) {
        tokens.push_back(__value->substr(first, last - first));
        first = last + __delimiter->size();
    }
    tokens.push_back(__value->substr(first));
    
    return tokens;
}


std::vector<std::string>
internal::split(
    const std::string* __value,
    std::string        __delimiter)
{
    // alias for c-string literals
    return split(__value, &__delimiter);
}


std::vector<std::string>
internal::split(
    const std::string* __value,
    std::regex         __delimiter)
{
    if (__value == NULL) return {};
    std::sregex_token_iterator next(
        __value->begin(), __value->end(), __delimiter, -1);
    std::sregex_token_iterator end;
    std::vector<std::string> tokens;
    std::string token;
    while (next != end) {
        token.assign(*next++);
        if (token.size())
            tokens.push_back(std::move(token));
    }
    return tokens;
}


std::vector<std::string>
internal::split(
    const std::string* __value,
    char               __delimiter,
    char               __escape)
{
    if (__value == NULL) return {};
    std::vector<std::string> tokens;
    size_t first = 0;
    size_t last  = __value->size();
    for (size_t next = first; next < last; next++) {
        if ((*__value)[next] == __escape) next++; // escape next char
        else if ((*__value)[next] == __delimiter) {
            tokens.push_back(__value->substr(first, next - first));
            first = next + 1;
        }
    }
    if (first < last) tokens.push_back(__value->substr(first, last - first));
    return tokens;
}
