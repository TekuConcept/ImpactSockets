/**
 * Created by TekuConcept on December 28, 2018
 */

#include <iomanip>
#include <sstream>
#include "utils/impact_error.h"
#include "utils/abnf_ops.h"
#include "rfc/http/abnf_ops.h"
#include "rfc/http/TX/response_message.h"

using namespace impact;
using namespace http;

#define HTAB(c)     impact::internal::HTAB(c)
#define SP(c)       impact::internal::SP(c)
#define VCHAR(c)    impact::internal::VCHAR(c)
#define OBS_TEXT(c) impact::http::internal::OBS_TEXT(c)

response_message::response_message(
    int         __status_code,
    std::string __reason_phrase)
: message(), m_status_code_(__status_code),
  m_reason_phrase_(__reason_phrase)
{
    if (__status_code > 999 || __status_code < 0)
        throw impact_error("Status code out of range");

    if (!_M_valid_phrase(__reason_phrase))
        throw impact_error("Bad reason phrase");
}


response_message::response_message(enum status_code __status_code)
{
    m_status_code_   = (int)__status_code;
    m_reason_phrase_ = status_code_string(__status_code);
}


response_message::~response_message()
{}


message_type
response_message::type() const
{
    return message_type::RESPONSE;
}


std::string
response_message::_M_start_line()
{
    std::ostringstream os;
    os << "HTTP/" << m_http_major_ << "." << m_http_minor_ << " ";
    os << std::setfill('0') << std::setw(3);
    os << m_status_code_ << std::setfill(' ');
    os << " " << m_reason_phrase_;
    return os.str();
}


bool
response_message::_M_valid_phrase(const std::string& __data) const
{
    // reason-phrase = *( HTAB / SP / VCHAR / obs-text )
    for (char c : __data)
        if (!HTAB(c) && !SP(c) && !VCHAR(c) && !OBS_TEXT(c)) return false;
    return true;
}
