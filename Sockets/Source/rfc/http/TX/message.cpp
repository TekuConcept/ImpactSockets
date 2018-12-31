/**
 * Created by TekuConcept on December 28, 2018
 */

#include <iomanip>
#include <sstream>
#include "utils/environment.h"
#include "utils/impact_error.h"
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
    m_header_ = header_token(field_name::TRANSFER_ENCODING, field_value.str());
}


transfer_encoding_token::~transfer_encoding_token()
{}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   Generic Message Constructors
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


message::message(message_traits_ptr __traits)
{ _M_initialize(&__traits, NULL, NULL); }


message::message(
    message_traits_ptr      __traits,
    transfer_encoding_token __data)
{ _M_initialize(&__traits, &__data, NULL); }


message::message(
    message_traits_ptr __traits,
    std::string        __data)
{ _M_initialize(&__traits, NULL, &__data); }


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   Request Message Convinience Constructors
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


message::message(
    std::string __method,
    std::string __target)
{
    message_traits_ptr traits((message_traits*)
        (new request_traits(__method, __target)));
    _M_initialize(&traits, NULL, NULL);
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
    _M_initialize(&traits, &data, NULL);
}


message::message(
    method_token            __method,
    target_token            __target,
    transfer_encoding_token __data)
{
    message_traits_ptr traits((message_traits*)
        (new request_traits(__method, __target)));
    _M_initialize(&traits, &__data, NULL);
}


message::message(
    std::string __method,
    std::string __target,
    std::string __data)
{
    message_traits_ptr traits((message_traits*)
        (new request_traits(__method, __target)));
    _M_initialize(&traits, NULL, &__data);
}


message::message(
    method_token __method,
    target_token __target,
    std::string  __data)
{
    message_traits_ptr traits((message_traits*)
        (new request_traits(__method, __target)));
    _M_initialize(&traits, NULL, &__data);
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
    _M_initialize(&traits, NULL, NULL);
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
    _M_initialize(&traits, &data, NULL);
}


message::message(
    int                     __status_code,
    std::string             __reason_phrase,
    transfer_encoding_token __data)
{
    message_traits_ptr traits((message_traits*)
        (new response_traits(__status_code, __reason_phrase)));
    _M_initialize(&traits, &__data, NULL);
}


message::message(
    int         __status_code,
    std::string __reason_phrase,
    std::string __data)
{
    message_traits_ptr traits((message_traits*)
        (new response_traits(__status_code, __reason_phrase)));
    _M_initialize(&traits, NULL, &__data);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   Member Functions
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


void
message::_M_initialize(
    message_traits_ptr*      __traits,
    transfer_encoding_token* __dynamic,
    std::string*             __fixed)
{
    if (*__traits == nullptr)
        throw impact_error("message_traits cannot be NULL");
    else m_traits_ = *__traits;
    
    if (__dynamic && __dynamic->callback &&
        m_traits_->permit_length_header()) {
        m_has_body_      = m_traits_->permit_body();
        m_is_fixed_body_ = false;
        m_data_          = *__dynamic;
        m_data_buffer_   = "";
        m_headers_.push_back(m_data_.header());
    }
    else if (__fixed && m_traits_->permit_length_header()) {
        std::ostringstream os;
        os << std::setfill('0') << std::setw(1) << __fixed->size();
        header_token header(field_name::CONTENT_LENGTH, os.str());
        m_headers_.push_back(header);
        m_has_body_      = m_traits_->permit_body();
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
    __stream << m_traits_->start_line();
    
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


const std::vector<header_token>&
message::headers() const noexcept
{
    return m_headers_;
}


void
message::headers(std::vector<header_token> __list)
{
    const int k_erase_length_headers = 0;
    const int k_throw_on_duplicate = 1;
    
    std::vector<header_token> next;
    int state = k_erase_length_headers;
    int count = 0;

    next.reserve(__list.size() + 1);
    if (m_has_body_) {
        if (m_traits_->permit_length_header()) {
            if (m_is_fixed_body_) {
                std::ostringstream os;
                os << std::setfill('0') << std::setw(1) << m_data_buffer_.size();
                header_token header(field_name::CONTENT_LENGTH, os.str());
                next.push_back(header);
            }
            else next.push_back(m_data_.header());
        }
    }
    else if (m_traits_->permit_user_length_header())
        state = k_throw_on_duplicate;
    
    for (const auto& header : __list) {
        if (header.field_name() == "Transfer-Encoding" ||
            header.field_name() == "Content-Length") {
            if (state == k_erase_length_headers) continue;
            else {
                count++;
                if (count > 1) {
                    throw impact_error("Duplicate content-length "
                        "or transfer_encoding headers");
                }
                else next.push_back(header);
            }
        }
        else next.push_back(header);
    }
    
    m_headers_ = next;
}
