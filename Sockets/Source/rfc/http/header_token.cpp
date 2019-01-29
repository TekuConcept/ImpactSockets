/**
 * Created by TekuConcept on December 28, 2018
 */

#include <iostream>
#include <regex>
#include "rfc/http/header_token.h"
#include "rfc/http/abnf_ops.h"
#include "utils/abnf_ops.h"
#include "utils/string_ops.h"
#include "utils/impact_error.h"

using namespace impact;
using namespace http;

#define is_token(str) impact::http::internal::is_token(str)
#define VCHAR(c)      impact::internal::VCHAR(c)
#define SP(c)         impact::internal::SP(c)
#define HTAB(c)       impact::internal::HTAB(c)


header_token::header_token(std::string __line)
{
    const std::string grammar_error =
        "line does not follow grammar rules";
    /* match indicies
        0 - full string match
        1 - field name
        2 - field value (trimmed)
    */
    const std::regex k_header_field_regex(
        "([!#$%&'*+\\-.^_`|~0-9a-zA-Z]+):"
        "[ \\t]*((?:[!-~]+)?(?:[ \\t]+[!-~]+)*)[ \\t]*\\r\\n"
    );
    /* match indicies
        0 - full string match
        1 - field value (trimmed)
    */
    const std::regex k_obs_fold_regex(
        "[ \\t]+((?:[!-~]+)?(?:[ \\t]+[!-~]+)*)[ \\t]*\\r\\n"
    );
    
    if (__line.size() == 0)
        throw impact_error("no value");
    
    auto end = __line.find_first_of("\r\n");
    if (end == std::string::npos)
        throw impact_error(grammar_error);
    end += 2;
    
    std::string line = __line.substr(0, end);
    std::smatch match;
    if (std::regex_match(line, match, k_header_field_regex)) {
        if (match.size() != 3) throw impact_error("regex parser error");
        auto name = match[1].str();
        m_field_name_.assign(name.c_str(), name.size());
        m_field_value_.assign(match[2].str());
    }
    else throw impact_error(grammar_error);
    
    while (end <  __line.size()) {
        auto begin = end;
        end = __line.find_first_of("\r\n", begin);
        if (end == std::string::npos)
            throw impact_error(grammar_error);
        end += 2;
        line = __line.substr(begin, end);
        if (std::regex_match(line, match, k_obs_fold_regex)) {
            if (match.size() != 2) throw impact_error("regex parser error");
            m_field_value_.append(" " + match[1].str());
        }
        else throw impact_error(grammar_error);
    }
}


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
    
    impact::internal::trim_whitespace(&m_field_value_);
    if (!_M_valid_value(m_field_value_))
        throw impact_error("Invalid value \"" + __value + "\"");
}


header_token::header_token(
    enum field_name  __id,
    std::string      __value)
{
    impact::internal::trim_whitespace(&__value);
    if (!_M_valid_value(__value))
        throw impact_error("Invalid value \"" + __value + "\"");
    m_field_value_ = __value;
    std::string temp = field_name_string(__id);
    m_field_name_.assign(temp.c_str(), temp.size());
}


header_token::~header_token()
{}


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
