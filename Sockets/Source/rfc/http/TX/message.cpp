/**
 * Created by TekuConcept on December 28, 2018
 */

#include <iomanip>
#include <sstream>
#include "rfc/http/TX/message.h"
#include "rfc/http/TX/request_message.h"
#include "rfc/http/TX/response_message.h"

using namespace impact;
using namespace http;


transfer_encoding_token::transfer_encoding_token(
    transfer_encoding_list __encodings)
: transfer_encoding_token(__encodings, nullptr)
{}


transfer_encoding_token::transfer_encoding_token(
    transfer_encoding_list            __encodings,
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
    m_header_ = message_header("Transfer-Encoding", field_value.str());
}


transfer_encoding_token::~transfer_encoding_token()
{}


message::message(message_type __type)
: m_type_(__type), m_http_major_(1), m_http_minor_(1),
  m_has_body_(false), m_is_fixed_body_(false), m_data_({},nullptr)
{}


message::message(
    message_type            __type,
    transfer_encoding_token __data)
: m_type_(__type), m_http_major_(1), m_http_minor_(1),
  m_has_body_(true), m_is_fixed_body_(false), m_data_(__data)
{
    if (!m_data_.callback) m_has_body_ = false;
    else m_headers_.push_back(m_data_.header());
}


message::~message()
{}


message_type
message::type() const noexcept
{
    return m_type_;
}


void
message::send(std::ostream& __stream)
{
    if (this->m_type_ == message_type::REQUEST) {
        request_message* start_line = dynamic_cast<request_message*>(this);
        __stream << start_line->method() << " ";
        __stream << start_line->target() << " ";
        __stream << "HTTP/" << m_http_major_ << "." << m_http_minor_ << "\r\n";
    }
    else /* (this->m_type_ == message_type::RESPONSE) */ {
        response_message* start_line = dynamic_cast<response_message*>(this);
        __stream << "HTTP/" << m_http_major_ << "." << m_http_minor_ << " ";
        __stream << std::setfill('0') << std::setw(3);
        __stream << start_line->code() << std::setfill(' ') << " ";
        __stream << start_line->status() << "\r\n";
    }
    
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
