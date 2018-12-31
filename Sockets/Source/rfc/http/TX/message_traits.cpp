/**
 * Created by TekuConcept on December 30, 2018
 */

#include <iomanip>
#include <sstream>
#include "utils/impact_error.h"
#include "utils/abnf_ops.h"
#include "rfc/uri.h"
#include "rfc/http/abnf_ops.h"
#include "rfc/http/TX/message_traits.h"

using namespace impact;
using namespace http;

#define HTAB(c)     impact::internal::HTAB(c)
#define SP(c)       impact::internal::SP(c)
#define VCHAR(c)    impact::internal::VCHAR(c)
#define OBS_TEXT(c) impact::http::internal::OBS_TEXT(c)

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   method_token
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

method_token::method_token(std::string __method_name)
{
    if (!_M_valid_name(__method_name))
        throw impact_error("Invalid method name");
    m_name_ = __method_name;
}


method_token::method_token(method __id)
{
    m_name_ = method_string(__id);
}


method_token::~method_token()
{}


bool
method_token::_M_valid_name(const std::string& __method) const
{
    return internal::is_token(__method);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   target_token
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

target_token::target_token(std::string __target_name)
{
    if ((m_type_ = _M_valid_target(__target_name)) == path_type::UNKNOWN)
        throw impact_error("Invalid target path");
    m_target_ = __target_name;
}


target_token::~target_token()
{}


target_token::path_type
target_token::_M_valid_target(const std::string& __target) const
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

message_traits::~message_traits()
{}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   request_traits
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

request_traits::request_traits(
    std::string __method,
    std::string __target)
: m_method_(method_token(__method)),
  m_target_(target_token(__target))
{
    m_http_major_ = 1;
    m_http_minor_ = 1;
}


request_traits::request_traits(
    method_token __method,
    target_token __target)
: m_method_(__method),
  m_target_(__target)
{
    m_http_major_ = 1;
    m_http_minor_ = 1;
}


request_traits::~request_traits()
{}


message_type
request_traits::type() const noexcept
{
    return message_type::REQUEST;
}


std::string
request_traits::start_line() const noexcept
{
    std::ostringstream os;
    os << m_method_.name() << " " << m_target_.name() << " ";
    os << "HTTP/" << m_http_major_ << "." << m_http_minor_;
    return os.str();
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   response_traits
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

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


message_type
response_traits::type() const noexcept
{
    return message_type::RESPONSE;
}


std::string
response_traits::start_line() const noexcept
{
    std::ostringstream os;
    os << "HTTP/" << m_http_major_ << "." << m_http_minor_ << " ";
    os << std::setfill('0') << std::setw(3);
    os << m_status_code_ << std::setfill(' ');
    os << " " << m_reason_phrase_;
    return os.str();
}
