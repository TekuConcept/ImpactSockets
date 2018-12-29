/**
 * Created by TekuConcept on December 28, 2018
 */

#include "utils/abnf_ops.h"
#include "rfc/http/abnf_ops.h"

#define HTAB(c)  impact::internal::HTAB(c)
#define SP(c)    impact::internal::SP(c)
#define VCHAR(c) impact::internal::VCHAR(c)

using namespace impact;

bool
http::internal::is_token(const std::string& __data)
{
    if (__data.size() == 0) return false;
    for (char c : __data)
        if (!TCHAR(c)) return false;
    return true;
}


bool
http::internal::is_quoted_string(const std::string& __data)
{
    if (__data.size() < 2) return false;
    if (__data[0] != '"' && __data[__data.size() - 1] != '"') return false;
    
    size_t start     = 1;
    size_t end       = __data.size() - 1;
    bool quoted_pair = false;
    
    char c;
    for (size_t i = start; i < end; i++) {
        c = __data[i];
        if (quoted_pair) {
            if (!HTAB(c) && !SP(c) && !VCHAR(c) && !OBS_TEXT(c)) return false;
            quoted_pair = false; // reset
        }
        else {
            if (c == '\\') quoted_pair = true;
            else {
                if (!HTAB(c) && !SP(c) && (c != '\x21') &&
                    !(c >= '\x23' && c <= '\x5B') &&
                    !(c >= '\x5D' && c <= '\x7E') && !OBS_TEXT(c)) return false;
            }
        }
    }
    if (quoted_pair) return false; // handle hanging '\'
    return true;
}
