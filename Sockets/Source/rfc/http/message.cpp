/**
 * Created by TekuConcept on October 3, 2019
 */

#include "rfc/http/message.h"

#include <sstream>

#include "utils/impact_error.h"

using namespace impact;
using namespace http;

message_t::limits::limits()
: max_line_length(8000),      // 8 kB
  max_header_limit(50),       // 50 header lines
  payload_size_limit(1000000) // 1 MB
{}


namespace impact {
namespace http {

std::ostream&
operator<<(
    std::ostream&    __os,
    const message_t& __message)
{
    __os << __message.to_string();
    return __os;
}

}}


message_t
message_t::get(std::string __target)
{
    message_t result;
    result.m_traits_ = message_traits_ptr(
        new request_traits(
            message_traits::method_t(method::GET),
            message_traits::target_t(__target))
    );
    return result;
}


message_t
message_t::post(std::string __target)
{
    message_t result;
    result.m_traits_ = message_traits_ptr(
        new request_traits(
            message_traits::method_t(method::POST),
            message_traits::target_t(__target))
    );
    return result;
}


message_t::message_t() = default;


message_t::message_t(
    message_traits_ptr __traits,
    header_list        __headers)
: m_traits_(__traits), m_headers_(__headers)
{ }


message_t::message_t(
    std::string __method,
    std::string __target,
    header_list __headers)
: m_headers_(__headers)
{
    try {
        m_traits_ = message_traits_ptr(
            new request_traits(__method, __target));
    }
    catch (...) { throw; }
}


message_t::message_t(
    method      __method,
    std::string __target,
    header_list __headers)
: m_headers_(__headers)
{
    try {
        m_traits_ = message_traits_ptr(
            new request_traits(
                message_traits::method_t(__method),
                message_traits::target_t(__target))
        );
    }
    catch (...) { throw; }
}


message_t::message_t(
    int         __status_code,
    std::string __reason_phrase,
    header_list __headers)
: m_headers_(__headers)
{
    try {
        m_traits_ = message_traits_ptr(
            new response_traits(__status_code, __reason_phrase));
    }
    catch (...) { throw; }
}


message_t::message_t(
    status_code __status_code,
    header_list __headers)
: m_headers_(__headers)
{
    try {
        m_traits_ = message_traits_ptr(
            new response_traits(__status_code));
    }
    catch (...) { throw; }
}


std::string
message_t::to_string() const
{
    /* HTTP-message ABNF
       HTTP-message = start-line
                      *( header-field CRLF )
                      CRLF
                      [ message-body ]
    */

    // 0: none
    // 1: content-length
    // 2: transfer-encoding
    // 3: not allowed
    int body_format = 0;
    std::ostringstream os;
    os << m_traits_->to_string();

    for (const auto& header : m_headers_.m_headers_) {
        if (header.m_describes_body_size_) {
            // overwrite or inject header values
            if (/* transfer-encoding= */NULL)
                 body_format = 2;
            else body_format = 1;
        }
        else os << header;
    }

    body_format = m_traits_->permit_length_header() ?
        body_format : 3;
    if (body_format < 2 && m_body_.size() > 0) {
        header_t header(
            field_name::CONTENT_LENGTH,
            std::to_string(m_body_.size()));
        body_format = 1;
        os << header;
    }
    else if (body_format == 2) {
        // generate Transfer-Encoding header
        // concatenate encoder types in order of application
    }

    os << "\r\n";

    if (m_traits_->permit_body()) {
        if (body_format == 1)
            os << m_body_;
        else if (body_format == 2) {
            // TODO: transfer encoding
        }
    }

    return os.str();
}
