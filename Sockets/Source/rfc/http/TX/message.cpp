/**
 * Created by TekuConcept on December 28, 2018
 */

#include <iomanip>
#include <sstream>
#include "rfc/http/TX/message.h"

using namespace impact;
using namespace http;


transfer_encoding_token::transfer_encoding_token()
: m_header_("_","")
{}


transfer_encoding_token::transfer_encoding_token(
    list                              __encodings,
    std::function<void(std::string*)> __callback)
: callback(__callback), m_header_("_", ""),
  m_transfer_encodings_(__encodings)
{
    bool have_chunked = false;
    for (size_t i = 0; i < m_transfer_encodings_.size(); i++) {
        if (m_transfer_encodings_[i]->name() == "chunked") {
            if (i == (m_transfer_encodings_.size() - 1))
                have_chunked = true;
            else {
                m_transfer_encodings_.erase(m_transfer_encodings_.begin() + i);
                i--;
            }
        }
    }
    if (!have_chunked)
        m_transfer_encodings_.push_back(transfer_encoding::chunked());
    std::ostringstream field_value;
    size_t i = 0;
    for (const auto& token : m_transfer_encodings_) {
        if (i > 0) field_value << ", ";
        field_value << token->name();
        i++;
    }
    m_header_ = header_token("Transfer-Encoding", field_value.str());
}


transfer_encoding_token::~transfer_encoding_token()
{}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   Generic Message Constructors
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


message::message(message_traits_ptr __traits)
{ _M_initialize(__traits, NULL, NULL); }


message::message(
    message_traits_ptr      __traits,
    transfer_encoding_token __data)
{ _M_initialize(__traits, &__data, NULL); }


message::message(
    message_traits_ptr __traits,
    std::string        __data)
{ _M_initialize(__traits, NULL, &__data); }


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   Request Message Convinience Constructors
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


message::message(
    std::string __method,
    std::string __target)
{
    message_traits_ptr traits((message_traits*)
        (new request_traits(__method, __target)));
    _M_initialize(traits, NULL, NULL);
}


message::message(
    std::string                       __method,
    std::string                       __target,
    transfer_encoding_token::list     __encodings,
    std::function<void(std::string*)> __data_callback)
{
    message_traits_ptr traits((message_traits*)
        (new request_traits(__method, __target)));
    transfer_encoding_token data(__encodings, __data_callback);
    _M_initialize(traits, &data, NULL);
}


message::message(
    method_token            __method,
    target_token            __target,
    transfer_encoding_token __data)
{
    message_traits_ptr traits((message_traits*)
        (new request_traits(__method, __target)));
    _M_initialize(traits, &__data, NULL);
}


message::message(
    std::string __method,
    std::string __target,
    std::string __data)
{
    message_traits_ptr traits((message_traits*)
        (new request_traits(__method, __target)));
    _M_initialize(traits, NULL, &__data);
}


message::message(
    method_token __method,
    target_token __target,
    std::string  __data)
{
    message_traits_ptr traits((message_traits*)
        (new request_traits(__method, __target)));
    _M_initialize(traits, NULL, &__data);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   Respons Message Convinience Constructors
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


message::message(
    int         __status_code,
    std::string __reason_phrase)
{
    message_traits_ptr traits((message_traits*)
        (new response_traits(__status_code, __reason_phrase)));
    _M_initialize(traits, NULL, NULL);
}


message::message(
    int                               __status_code,
    std::string                       __reason_phrase,
    transfer_encoding_token::list     __encodings,
    std::function<void(std::string*)> __data_callback)
{
    message_traits_ptr traits((message_traits*)
        (new response_traits(__status_code, __reason_phrase)));
    transfer_encoding_token data(__encodings, __data_callback);
    _M_initialize(traits, &data, NULL);
}


message::message(
    int                     __status_code,
    std::string             __reason_phrase,
    transfer_encoding_token __data)
{
    message_traits_ptr traits((message_traits*)
        (new response_traits(__status_code, __reason_phrase)));
    _M_initialize(traits, &__data, NULL);
}


message::message(
    int         __status_code,
    std::string __reason_phrase,
    std::string __data)
{
    message_traits_ptr traits((message_traits*)
        (new response_traits(__status_code, __reason_phrase)));
    _M_initialize(traits, NULL, &__data);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   Member Functions
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


void
message::_M_initialize(
    message_traits_ptr       __traits,
    transfer_encoding_token* __dynamic,
    std::string*             __fixed)
{
    m_traits_ = __traits;
    
    if (__dynamic && __dynamic->callback) {
        m_has_body_      = true;
        m_is_fixed_body_ = false;
        m_data_          = *__dynamic;
        m_data_buffer_   = "";
        m_headers_.push_back(m_data_.header());
    }
    else if (__fixed) {
        std::ostringstream os;
        os << std::setfill('0') << std::setw(1) << __fixed->size();
        header_token header("Content-Length", os.str());
        m_headers_.push_back(header);
        m_has_body_      = true;
        m_is_fixed_body_ = true;
        m_data_buffer_   = *__fixed;
    }
    else {
        m_has_body_      = false;
        m_is_fixed_body_ = false;
        m_data_buffer_   = "";
    }
}


message::~message()
{}


void
message::send(std::ostream& __stream)
{
    __stream << m_traits_->start_line() << "\r\n";
    
    for (const auto& header : m_headers_) {
        __stream << header.field_name() << ": ";
        __stream << header.field_value() << "\r\n";
    }
    
    __stream << "\r\n"; // end of HTTP headers
    
    if (!m_has_body_) return;
    if (m_is_fixed_body_) {
        __stream << m_data_buffer_;
    }
    else {
        do {
            m_data_buffer_.clear();
            m_data_.callback(&m_data_buffer_);
            std::string out_buffer = m_data_buffer_;
            for (const auto& encoding : m_data_.encodings())
                out_buffer = encoding->encode(out_buffer);
            __stream << out_buffer;
        } while(m_data_buffer_.size() > 0);
    }
}


message_type
message::type() const noexcept
{
    return m_traits_->type();
}
