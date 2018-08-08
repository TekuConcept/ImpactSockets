/**
 * Created by TekuConcept on July 28, 2018
 */

#include "rfc/utf8.h"

#include <sstream>

#include "utils/errno.h"


using namespace impact;

/*  NOTE:
    The serialize string functions use the same
    code snippet below but each use a different
    basic_string<T> type.
*/

#define SERIALIZE_STRING_CODE_SNIPPET(mask) {           \
    imp_errno = imperr::SUCCESS;                        \
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
        imp_errno = imperr::UTF8_BADSYM;                \
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
    imp_errno = imperr::SUCCESS;

    if (__input > 0x0010FFFF ||
        (__input >= 0x0000D800 && __input <= 0x0000DFFF)) {
        imp_errno = imperr::UTF8_BADSYM;
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
    imp_errno = imperr::SUCCESS;

    int state = 0;
    uint32_t symbol;
    for (char c : __input) {
        switch (state) {
        case 0: /* header */ {
            if ((c & 0x80) == 0x00) {
                symbol = c;
                if (__result) __result->push_back(symbol);
                continue;
            }
                 if ((c & 0xE0) == 0xC0) state = 1;
            else if ((c & 0xF0) == 0xE0) state = 2;
            else if ((c & 0xF8) == 0xF0) state = 3;
            else goto header_error;
            symbol = ((0x3F >> state) & c);
            continue;
        header_error:
            imp_errno = imperr::UTF8_BADHEAD;
            return false;
        } break;
        default: /* trailer */ {
            if ((c & 0xC0) == 0x80) {
                symbol <<= 6;
                symbol |= (0x3F & c);
                state--;
                if (state == 0 && __result)
                    __result->push_back(symbol);
            }
            else {
                imp_errno = imperr::UTF8_BADTRAIL;
                return false;
            }
        } break;
        }
    }

    return true;
}
