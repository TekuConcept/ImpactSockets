/**
 * Created by TekuConcept on July 28, 2018
 */

#include "rfc/utf8.h"

#include <sstream>

#include "sockets/impact_errno.h"


using namespace impact;

/*  NOTE:
    The serialize string functions use the same
    code snippet below but each use a different
    basic_string<T> type.
*/

#define SERIALIZE_STRING_CODE_SNIPPET(mask) {           \
    impact_errno = SUCCESS;                             \
    size_t size = 0;                                    \
    for (auto& c : __input) {                           \
        size_t test = _S_estimate_buf_size( mask & c ); \
        if (test == static_cast<size_t>(-1)) {          \
            size = test;                                \
            break;                                      \
        }                                               \
        size += test;                                   \
    }                                                   \
    if (size == static_cast<size_t>(-1)) {              \
        impact_errno = UTF8_BADSYM;                     \
        return false;                                   \
    }                                                   \
    if (__result) {                                     \
        __result->reserve(size + __result->size());     \
        for (auto& symbol : __input)                    \
            _S_encode( mask & symbol, __result);        \
    }                                                   \
    return true;                                        \
}


bool
utf8::serialize(
    char32_t     __input,
    std::string* __result)
{
    impact_errno = SUCCESS;
    
    if (__input > 0x0010FFFF ||
        (__input >= 0x0000D800 && __input <= 0x0000DFFF)) {
        impact_errno = UTF8_BADSYM;
        return false;
    }
    
    if (__result) _S_encode(__input, __result);
    
    return true;
}


bool
utf8::serialize(
    const std::string& __input,
    std::string*       __result)
{
    SERIALIZE_STRING_CODE_SNIPPET(0x000000FF)
}


bool
utf8::serialize(
    const std::u16string& __input,
    std::string*          __result)
{
    SERIALIZE_STRING_CODE_SNIPPET(0x0000FFFF)
}


bool
utf8::serialize(
    const std::u32string& __input,
    std::string*          __result)
{
    SERIALIZE_STRING_CODE_SNIPPET(0xFFFFFFFF)
}


bool
utf8::deserialize(
    const std::string& __input,
    std::u32string*    __result)
{
    impact_errno = SUCCESS;
    (void)__result;
    
    int state = 0;
    uint32_t symbol;
    for (char c : __input) {
        switch (state) {
        case 0: /* header */ {
            if ((c & 0x80) == 0x00) {
                symbol = c;
                if (__result) __result->push_back(symbol);
            }
            else if ((c & 0xE0) == 0xC0) {
                symbol = (0x1F & c);
                state = 1; /* 1 trailer */
            }
            else if ((c & 0xF0) == 0xE0) {
                symbol = (0x0F & c);
                state = 2; /* 2 trailers */
            }
            else if ((c & 0xF8) == 0xF0) {
                symbol = (0x07 & c);
                state = 3; /* 3 trailers */
            }
            else {
                impact_errno = UTF8_BADSYM;
                return false;
            }
        } break;
        default: /* trailer */ {
            if ((c & 0xC0) == 0x80) {
                symbol <<= 6;
                symbol |= (0x3F & c);
                state--;
                if (state == 0 && __result) {
                    __result->push_back(symbol);
                }
            }
            else {
                impact_errno = UTF8_BADSYM;
                return false;
            }
        } break;
        }
    }
    
    return true;
}
