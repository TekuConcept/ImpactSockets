/**
 * Created by TekuConcept on December 28, 2018
 */

#include <iostream>
#include <sstream>
#include <regex>
#include <algorithm>

#include "rfc/http/header.h"
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


namespace impact {
namespace http {

std::ostream&
operator<<(
    std::ostream&   __os,
    const header_t& __header)
{
    __os << __header.to_string();
    return __os;
}

}}


header_t::header_t(std::string __line)
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
            if (match.size() != 2)
                throw impact_error("regex parser error");
            else m_field_value_.append(" " + match[1].str());
        }
        else throw impact_error(grammar_error);
    }

    m_describes_body_size_ = (
        m_field_name_ == "Content-Length" ||
        m_field_name_ == "Transfer-Encoding"
    );
}


header_t::header_t(
    std::string __name,
    std::string __value)
: m_field_name_(__name.c_str(), __name.size()),
  m_field_value_(__value)
{
    // header-field   = field-name ":" OWS field-value OWS
    // field-name     = token
    // field-value    = *( VCHAR [ 1*( SP / HTAB ) VCHAR ] )

    if (!is_token(__name))
        throw impact_error("Invalid name \"" + __name + "\"");
    
    impact::internal::trim_whitespace(&m_field_value_);
    if (!_M_valid_value(m_field_value_))
        throw impact_error("Invalid value \"" + __value + "\"");
    
    m_describes_body_size_ = (
        m_field_name_ == "Content-Length" ||
        m_field_name_ == "Transfer-Encoding"
    );
}


header_t::header_t(
    enum field_name  __id,
    std::string      __value)
{
    impact::internal::trim_whitespace(&__value);
    if (!_M_valid_value(__value))
        throw impact_error("Invalid value \"" + __value + "\"");
    m_field_value_ = __value;
    std::string temp = field_name_string(__id);
    m_field_name_.assign(temp.c_str(), temp.size());

    m_describes_body_size_ = (
        __id == field_name::CONTENT_LENGTH ||
        __id == field_name::TRANSFER_ENCODING
    );
}


header_t::~header_t() = default;


std::string
header_t::to_string() const
{
    std::ostringstream os;
    os << m_field_name_ << ": " << m_field_value_ << "\r\n";
    return os.str();
}


void
header_t::append_value(std::string __value)
{
    if (!_M_valid_value(__value))
        throw impact_error("Invalid value \"" + __value + "\"");
    if (m_field_value_.size() > 0)
        m_field_value_ += ", ";
    impact::internal::trim_whitespace(&__value);
    m_field_value_ += __value;
}


void
header_t::append_value_from(const header_t& __header)
{
    if (m_field_value_.size() > 0)
        m_field_value_ += ", ";
    m_field_value_ += __header.m_field_value_;
}


bool
header_t::_M_valid_value(const std::string& __value) const
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


header_list::header_list()
: m_has_body_size_(false)
{}


header_list::header_list(std::initializer_list<header_t> __headers)
: m_has_body_size_(false)
{
    for (auto it = __headers.begin(); it != __headers.end(); it++)
        _M_insert(m_headers_.end(), *it);
}


header_list::header_list(const std::vector<header_t>& __headers)
: m_has_body_size_(false)
{
    for (const auto& header : __headers)
        _M_insert(m_headers_.end(), header);
}


std::vector<header_t>::const_iterator
header_list::insert(
    std::vector<header_t>::const_iterator __pos,
    const header_t&                       __value)
{ return _M_insert(__pos, __value); }


std::vector<header_t>::const_iterator
header_list::insert(
    std::vector<header_t>::const_iterator __pos,
    header_t&&                            __value)
{ return _M_insert(__pos, __value); }


std::vector<header_t>::const_iterator
header_list::insert(
    std::vector<header_t>::const_iterator __pos,
    std::vector<header_t>::const_iterator __first,
    std::vector<header_t>::const_iterator __last )
{
    bool merged;
    auto position = __pos;
    for (auto it = __first; it != __last; it++) {
        auto next_position = _M_insert(position, *it, &merged);
        if (!merged)
            position = ++next_position;
    }
    return position;
}


std::vector<header_t>::const_iterator
header_list::insert(
    std::vector<header_t>::const_iterator __pos,
    std::initializer_list<header_t>       __ilist )
{
    bool merged;
    auto position = __pos;
    for (auto it = __ilist.begin(); it != __ilist.end(); it++) {
        auto next_position = _M_insert(position, *it, &merged);
        if (!merged)
            position = ++next_position;
    }
    return position;
}


std::vector<header_t>::const_iterator
header_list::erase(
    std::vector<header_t>::const_iterator __pos )
{
    if (__pos != m_headers_.end() &&
        __pos->m_describes_body_size_)
        m_has_body_size_ = false;
    return m_headers_.erase(__pos);
}


std::vector<header_t>::const_iterator
header_list::erase(
    std::vector<header_t>::const_iterator __first,
    std::vector<header_t>::const_iterator __last )
{
    for (auto it = __first; it != __last; it++) {
        if (it->m_describes_body_size_) {
            m_has_body_size_ = false;
            break;
        }
    }
    return m_headers_.erase(__first, __last);
}


void
header_list::push_back(const header_t& __header)
{ _M_insert(m_headers_.end(), __header); }


void
header_list::push_back(header_t&& __header)
{ _M_insert(m_headers_.end(), __header); }


void
header_list::pop_back()
{
    if (m_headers_.size() == 0)
        m_has_body_size_ = false;
    else if (m_headers_.back().m_describes_body_size_)
        m_has_body_size_ = false;
    m_headers_.pop_back();
}


std::vector<header_t>::const_iterator
header_list::_M_insert(
    std::vector<header_t>::const_iterator __position,
    const header_t&                       __header,
    bool*                                 __merged)
{
    // - "Transfer-Encoding" and "Content-Length"
    // duplicates not allowed
    // - "Set-Cookie" duplicates allowed
    // - all other duplicates need to be concatenated

    if (m_has_body_size_ && __header.m_describes_body_size_)
        throw impact_error("cannot have both"
            " transfer-encoding and content-length headers"
            " nor duplicates thereof");
    else m_has_body_size_ |= __header.m_describes_body_size_;

    bool merged = false;
    auto result = std::find_if(
        m_headers_.begin(),
        m_headers_.end(),
        [&](const header_t& __token)
        { return __token.name() == __header.name(); }
    );

    if (result == m_headers_.end())
        return m_headers_.insert(__position, __header);
    else {
        if (__header.name() == "Set-Cookie")
            return m_headers_.insert(__position, __header);
        else {
            result->append_value_from(__header);
            merged = true;
        }
    }

    if (__merged) *__merged = merged;
    return __position;
}
