/**
 * Created by TekuConcept on December 28, 2018
 */

#ifndef _IMPACT_HTTP_ABNF_OPS_H_
#define _IMPACT_HTTP_ABNF_OPS_H_

#include <string>

namespace impact {
namespace http {
namespace internal {
    inline bool
    TCHAR(char c)
    {
        if (c >= 'A' && c <= 'Z')  return true;
        if (c >= '^' && c <= 'z')  return true; // ^, _, `, a-z
        if (c >= '0' && c <= '9')  return true;
        if (c >= '#' && c <= '\'') return true; // #, $, %, &, '
        if (c == '!' || c == '*' || c == '+' || c == '-' ||
            c == '.' || c == '|' || c == '~') return true;
        return false;
    }
    
    inline bool OBS_TEXT(char c) { return ((c >= '\x80' && c <= '\xFE') || (c == '\xFF')); }
    
    bool is_token(const std::string& data);
    bool is_quoted_string(const std::string& data);
}}}

#endif
