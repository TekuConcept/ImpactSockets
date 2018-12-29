/**
 * Created by TekuConcept on December 28, 2018
 */

#include "rfc/http/TX/header_token.h"
#include "rfc/http/abnf_ops.h"
#include "utils/abnf_ops.h"
#include "utils/impact_error.h"

using namespace impact;
using namespace http;

#define is_token(str) impact::http::internal::is_token(str)
#define VCHAR(c)      impact::internal::VCHAR(c)
#define SP(c)         impact::internal::SP(c)
#define HTAB(c)       impact::internal::HTAB(c)

header_token::header_token(
    std::string __name,
    std::string __value)
: m_field_name_(__name.c_str(), __name.size()), m_field_value_(__value)
{
    // header-field   = field-name ":" OWS field-value OWS
    // field-name     = token
    // field-value    = *( VCHAR [ 1*( SP / HTAB ) VCHAR ] )

    if (!is_token(__name))
        throw impact_error("Invalid name \"" + __name + "\"");
    
    _M_trim_whitespace(&m_field_value_);
    if (!_M_valid_value(m_field_value_))
        throw impact_error("Invalid value \"" + __value + "\"");
}


header_token::header_token(
    enum field_name  __id,
    std::string      __value)
{
    _M_trim_whitespace(&__value);
    if (!_M_valid_value(__value))
        throw impact_error("Invalid value \"" + __value + "\"");
    m_field_value_ = __value;
    std::string temp = field_name_string(__id);
    m_field_name_.assign(temp.c_str(), temp.size());
}


header_token::~header_token()
{}


void
header_token::_M_trim_whitespace(std::string* __data)
{
    const std::string k_whitespace = " \t";
    
    size_t begin = __data->find_first_not_of(k_whitespace);
    if (begin == std::string::npos) return;
    size_t end = __data->find_last_not_of(k_whitespace);
    size_t length = end - begin + 1;
    if (length == __data->size()) return;
    *__data = __data->substr(begin, length);
}


bool
header_token::_M_valid_value(const std::string& __value) const
{
    if (__value.size() == 0) return true;
    if (!VCHAR(__value[0])) return false;
    char c;
    for (size_t i = 1; i < __value.size(); i++) {
        c = __value[i];
        if (!VCHAR(c) && !SP(c) && !HTAB(c)) return false;
    }
    return true;
}
