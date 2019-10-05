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
    result.m_traits_.reset(
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
    result.m_traits_.reset(
        new request_traits(
            message_traits::method_t(method::POST),
            message_traits::target_t(__target))
    );
    return result;
}


message_t::message_t() = default;


message_t::message_t(
    std::unique_ptr<message_traits> __traits,
    header_list                     __headers,
    std::string                     __body)
: m_traits_(std::move(__traits)),
  m_headers_(__headers),
  m_body_(__body)
{ }


message_t::message_t(
    std::string __method,
    std::string __target,
    header_list __headers,
    std::string __body)
: m_headers_(__headers), m_body_(__body)
{
    try { m_traits_.reset(new request_traits(__method, __target)); }
    catch (...) { throw; }
}


message_t::message_t(
    method      __method,
    std::string __target,
    header_list __headers,
    std::string __body)
: m_headers_(__headers), m_body_(__body)
{
    try {
        m_traits_.reset(new request_traits(
                message_traits::method_t(__method),
                message_traits::target_t(__target))
        );
    }
    catch (...) { throw; }
}


message_t::message_t(
    int         __status_code,
    std::string __reason_phrase,
    header_list __headers,
    std::string __body)
: m_headers_(__headers), m_body_(__body)
{
    try {
        m_traits_.reset(new response_traits(__status_code, __reason_phrase));
    }
    catch (...) { throw; }
}


message_t::message_t(
    status_code __status_code,
    header_list __headers,
    std::string __body)
: m_headers_(__headers), m_body_(__body)
{
    try { m_traits_.reset(new response_traits(__status_code)); }
    catch (...) { throw; }
}


message_t::message_t(message_t&& __rhs)
{
    this->m_traits_  = std::move(__rhs.m_traits_);
    this->m_pipe_    = std::move(__rhs.m_pipe_);
    this->m_headers_ = std::move(__rhs.m_headers_);
    this->m_body_    = std::move(__rhs.m_body_);
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

    std::ostringstream os;
    os << m_traits_->to_string();

    for (const auto& header : m_headers_.m_headers_) {
        if (header.m_describes_body_size_)
            // overwrite or inject header values
            continue;
        else os << header;
    }

    if (m_traits_->permit_length_header()) {
        if (m_pipe_ != nullptr) {
            os << field_name_string(field_name::TRANSFER_ENCODING);
            os << ": ";
            bool first = true;
            for (const auto& coding : m_pipe_->codings()) {
                if (!first) os << ", ";
                os << coding->name();
                first = false;
            }
            os << "\r\n";
        }
        else if (m_body_.size() > 0) {
            header_t header(
                field_name::CONTENT_LENGTH,
                std::to_string(m_body_.size()));
            os << header;
        }
    }

    os << "\r\n";

    if (m_traits_->permit_body()) {
        if (m_pipe_ == nullptr && m_body_.size() > 0)
            os << m_body_;
    }

    return os.str();
}


message_t
message_t::clone() const
{
    message_t result;
    result.m_traits_  = this->m_traits_->clone();
    result.m_headers_ = this->m_headers_;
    result.m_body_    = this->m_body_;
    return result;
}
