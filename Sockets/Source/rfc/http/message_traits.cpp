/**
 * Created by TekuConcept on December 30, 2018
 */

#include <iomanip>
#include <sstream>
#include <regex>
#include "utils/impact_error.h"
#include "utils/abnf_ops.h"
#include "rfc/uri.h"
#include "rfc/http/abnf_ops.h"
#include "rfc/http/message_traits.h"

using namespace impact;
using namespace http;

#define HTAB(c)     impact::internal::HTAB(c)
#define SP(c)       impact::internal::SP(c)
#define VCHAR(c)    impact::internal::VCHAR(c)
#define OBS_TEXT(c) impact::http::internal::OBS_TEXT(c)


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   method_token
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


message_traits::method_t::method_t() = default;


message_traits::method_t::method_t(std::string __method_name)
{
    if (!_M_valid_name(__method_name))
        throw impact_error("Invalid method name");
    m_name_ = __method_name;
}


message_traits::method_t::method_t(http::method __id)
{ m_name_ = method_string(__id); }


message_traits::method_t::~method_t() = default;


bool
message_traits::method_t::_M_valid_name(const std::string& __method) const
{ return internal::is_token(__method); }


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   target_token
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


message_traits::target_t::target_t() = default;


message_traits::target_t::target_t(std::string __target_name)
{
    if ((m_type_ = _M_valid_target(__target_name)) == path_type::UNKNOWN)
        throw impact_error("Invalid target path");
    m_target_ = __target_name;
}


message_traits::target_t::~target_t() = default;


message_traits::target_t::path_type
message_traits::target_t::_M_valid_target(const std::string& __target) const
{
    if (__target.size() == 0) return path_type::UNKNOWN;
    if (__target == "*") return path_type::ASTERISK;
    if (__target[0] == '/') {
        if (__target.size() == 1) return path_type::ORIGIN;
        // don't accidentally mistake authority "//"
        else if (__target[1] == '/') return path_type::UNKNOWN;
        else {
            if (uri::parse_resource(__target, NULL))
                 return path_type::ORIGIN;
            else return path_type::UNKNOWN;
        }
    }
    else {
        bool valid = uri::parse_authority(__target, NULL);
        if (!valid) {
            if (uri::parse(__target, NULL))
                 return path_type::ABSOLUTE;
            else return path_type::UNKNOWN;
        }
        else return path_type::AUTHORITY;
    }
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   message_traits
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


message_traits_ptr
message_traits::create(std::string __line)
{
    /* match indicies
        0 - full string match
        1 - request match
        2 - request method
        3 - request target
        4 - request HTTP version
        5 - response match
        6 - response HTTP version
        7 - response status code
        8 - response status message
    */
    const std::regex k_start_line_regex(
        "(([!#$%&'*+\\-.^_`|~0-9a-zA-Z]+) ([a-zA-Z0-9+\\-.:\\/_~%!$&'\\(\\)*,;="
        "@\\[\\]?]+|\\*) (HTTP\\/[0-9]\\.[0-9])\\r\\n)|((HTTP\\/[0-9]\\.[0-9]) "
        "([0-9][0-9][0-9]) ([\\t \\x21-\\x7E\\x80-\\xFF]*)\\r\\n)");
    
    if (__line.size() == 0)
        throw impact_error("no value");
    
    message_traits_ptr result;
    
    std::smatch match;
    if (std::regex_match(__line, match, k_start_line_regex)) {
        // current assumptions are that given 9 matches,
        // - four of the matches are blank
        // - three subsequent match indicies following 1 or 5 are not blank
        if (match.size() != 9) throw impact_error("regex parser error");
        
        // typeof(match[i]) = std::ssub_match
        std::string http_version;
        if (match[1].str().size() != 0) {
            http_version               = match[4].str();
            request_traits* request    = new request_traits();
            request->m_method_.m_name_ = match[2].str();
            // full validation: regex validated target charset,
            // target_token will now validate formatting and determine the type
            request->m_target_         = message_traits::target_t(match[3].str());
            result = message_traits_ptr((message_traits*)request);
        }
        else if (match[5].str().size() != 0) {
            http_version               = match[6].str();
            response_traits* response  = new response_traits();
            response->m_status_code_   = std::stoi(match[7].str());
            response->m_reason_phrase_ = match[8].str();
            result = message_traits_ptr((message_traits*)response);
        }
        else throw impact_error("line does not follow grammar rules");
        
        result->m_http_major_ = http_version[5] - '0';
        result->m_http_minor_ = http_version[7] - '0';
    }
    else throw impact_error("line does not follow grammar rules");
    
    return result;
}


message_traits::~message_traits()
{}


namespace impact {
namespace http {

std::ostream&
operator<<(
    std::ostream&         __os,
    const request_traits& __traits)
{
    __os << __traits.to_string();
    return __os;
}


std::ostream&
operator<<(
    std::ostream&          __os,
    const response_traits& __traits)
{
    __os << __traits.to_string();
    return __os;
}

}}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   request_traits
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


request_traits::request_traits()
{}


request_traits::request_traits(
    std::string __method,
    std::string __target)
: m_method_(message_traits::method_t(__method)),
  m_target_(message_traits::target_t(__target))
{
    m_http_major_ = 1;
    m_http_minor_ = 1;
}


request_traits::request_traits(
    message_traits::method_t __method,
    message_traits::target_t __target)
: m_method_(__method),
  m_target_(__target)
{
    m_http_major_ = 1;
    m_http_minor_ = 1;
}


request_traits::~request_traits()
{}


message_traits::message_type
request_traits::type() const noexcept
{ return message_type::REQUEST; }


std::string
request_traits::to_string() const noexcept
{
    std::ostringstream os;
    os << m_method_.name() << " " << m_target_.name() << " ";
    os << "HTTP/" << m_http_major_ << "." << m_http_minor_;
    os << "\r\n";
    return os.str();
}


bool
request_traits::permit_user_length_header() const noexcept
{
    return (m_method_.name() == "HEAD");
}


bool
request_traits::permit_length_header() const noexcept
{ return true; }


bool
request_traits::permit_body() const noexcept
{
    return permit_length_header() &&
        (m_method_.name() != "HEAD") &&
        (m_method_.name() != "TRACE");
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   response_traits
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


response_traits::response_traits()
{}


response_traits::response_traits(
    int         __status_code,
    std::string __reason_phrase)
: m_status_code_(__status_code),
  m_reason_phrase_(__reason_phrase)
{
    _M_validate();
    m_http_major_ = 1;
    m_http_minor_ = 1;
}


response_traits::response_traits(enum status_code __id)
{
    m_status_code_   = (int)__id;
    m_reason_phrase_ = status_code_string(__id);
    m_http_major_    = 1;
    m_http_minor_    = 1;
}


void
response_traits::_M_validate()
{
    if (m_status_code_ > 999 || m_status_code_ < 0)
        throw impact_error("Status code out of range");
    if (!_M_valid_phrase(m_reason_phrase_))
        throw impact_error("Bad reason phrase");
}


bool
response_traits::_M_valid_phrase(const std::string& __data) const noexcept
{
    for (char c : __data)
        if (!HTAB(c) && !SP(c) && !VCHAR(c) && !OBS_TEXT(c)) return false;
    return true;
}


response_traits::~response_traits()
{}


message_traits::message_type
response_traits::type() const noexcept
{ return message_type::RESPONSE; }


std::string
response_traits::to_string() const noexcept
{
    std::ostringstream os;
    os << "HTTP/" << m_http_major_ << "." << m_http_minor_ << " ";
    os << std::setfill('0') << std::setw(3);
    os << m_status_code_ << std::setfill(' ');
    os << " " << m_reason_phrase_ << "\r\n";
    return os.str();
}


bool
response_traits::permit_user_length_header() const noexcept
{
    return (m_status_code_ == 304); /* Not Modified */
}


bool
response_traits::permit_length_header() const noexcept
{
    if (m_status_code_ >= 100 && m_status_code_ < 200)
        return false;
    else if (m_status_code_ == 204)
        return false;
    return true;
}


bool
response_traits::permit_body() const noexcept
{
    return permit_length_header() && (m_status_code_ != 304);
}
