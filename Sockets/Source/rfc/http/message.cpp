/**
 * Created by TekuConcept on December 28, 2018
 */

#include <iomanip>
#include <sstream>
#include "utils/environment.h"
#include "utils/impact_error.h"
#include "rfc/http/message.h"

#include <iostream>

using namespace impact;
using namespace http;


transfer_encoding_token::transfer_encoding_token()
: m_header_("_","")
{}


transfer_encoding_token::transfer_encoding_token(
    list                                __encodings,
    std::function<void(std::string*)>&& __callback)
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
    std::string                         __method,
    std::string                         __target,
    transfer_encoding_token::list       __encodings,
    std::function<void(std::string*)>&& __data_callback)
{
    message_traits_ptr traits((message_traits*)
        (new request_traits(__method, __target)));
    transfer_encoding_token data(__encodings, std::move(__data_callback));
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
    int                                 __status_code,
    std::string                         __reason_phrase,
    transfer_encoding_token::list       __encodings,
    std::function<void(std::string*)>&& __data_callback)
{
    message_traits_ptr traits((message_traits*)
        (new response_traits(__status_code, __reason_phrase)));
    transfer_encoding_token data(__encodings, std::move(__data_callback));
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
message::send(
    std::ostream&  __stream,
    const message& __message)
{
    __stream << __message.m_traits_->start_line();
    
    for (const auto& header : __message.m_headers_) {
        __stream << header.field_name() << ": ";
        __stream << header.field_value() << "\r\n";
    }
    
    __stream << "\r\n"; // end of HTTP headers
    
    if (!__message.m_has_body_) return;
    if (__message.m_is_fixed_body_) {
        __stream << __message.m_data_buffer_;
    }
    else {
        std::string data_buffer;
        size_t initial_size;
        do {
            data_buffer.clear();
            __message.m_data_.callback(&data_buffer);
            initial_size = data_buffer.size();
            for (const auto& encoding : __message.m_data_.encodings())
                data_buffer = encoding->encode(data_buffer);
            __stream << data_buffer;
        } while (initial_size > 0);
    }
}


#define MESSAGE_SUCCESS  0
#define MESSAGE_FAILED   1
#define VERSION_FAILED   2
#define MESSAGE_CONTINUE 3
#define HEADER_OVERLOAD  4
#define NO_BODY          0
#define FIXED_BODY       1
#define DYNAMIC_BODY     2


inline int
message::_M_process_start_line(
    std::istream&        __stream,
    std::string&         __buffer,
    const parser_limits& __limits,
    message_traits_ptr&  __traits)
{
    __buffer.resize(__limits.max_line_length, '\0');

    if (!__stream.getline(&__buffer[0], __buffer.size() - 1, '\n'))
        return MESSAGE_FAILED;
    auto result = __stream.gcount();

    if (result <= 0) return MESSAGE_FAILED;
    __buffer[result - 1] = '\n'; // getline discards this but it is needed

    try { __traits = message_traits::create(__buffer.substr(0, result)); }
    catch (...) { return MESSAGE_FAILED; }

    if (__traits->http_major() != 1 || __traits->http_minor() != 1)
        return VERSION_FAILED;
    return MESSAGE_SUCCESS;
}


inline int
message::_M_process_header_line(
    std::istream&        __stream,
    std::string&         __buffer,
    const parser_limits& __limits,
    unsigned int*        __body_info,
    header_list&         __headers)
{
    if (!__stream.getline(&__buffer[0], __buffer.size() - 1, '\n'))
        return MESSAGE_FAILED;
    auto result = __stream.gcount();
    
    if (result <= 1) return MESSAGE_FAILED;

    if (__buffer[0] == '\r') { return MESSAGE_SUCCESS; }
    else {
        if (__headers.size() >= __limits.max_header_limit)
            return HEADER_OVERLOAD;
        if (__buffer[result - 2] != '\r')
            return HEADER_OVERLOAD;
        else __buffer[result - 1] = '\n';

        try {
            __headers.push_back(header_token(__buffer.substr(0, result)));
            case_string name = __headers.back().field_name();
            if (name == "Content-Length") {
                if (__body_info[0] != NO_BODY) return MESSAGE_FAILED;
                __body_info[0] = FIXED_BODY;
                __body_info[1] = __headers.size() - 1;
            }
            else if (name == "Transfer-Encoding") {
                if (__body_info[0] != NO_BODY) return MESSAGE_FAILED;
                __body_info[0] = DYNAMIC_BODY;
                __body_info[1] = __headers.size() - 1;
            }
        }
        catch (...) { return MESSAGE_FAILED; }
    }

    return MESSAGE_CONTINUE;
}


inline int
message::_M_process_body(
    std::istream&        __stream,
    std::string&         __buffer,
    const parser_limits& __limits,
    const unsigned int*  __body_info,
    header_list&         __headers,
    data_recv_callback&  __callback)
{
    // const int k_max_int_string = 10; // 4294967295
    
    UNUSED(__stream);
    UNUSED(__buffer);
    UNUSED(__limits);
    UNUSED(__headers);
    UNUSED(__callback);
    
    // int header_index = __body_info[1];
    if (__body_info[0] == FIXED_BODY) {
        // std::string string_length = __headers[header_index].field_value;
        // // max unsigned long: 18446744073709551615
        // if (string_length.size() > k_max_int_string)
    }
    else /* dynamic */ {
        // - warning TE without chunked bodies are allowed for
        // response messages; end of connection signals end of body
        // - unrecognized TE results in 501 (Not Implemented) error
    }
    
    return MESSAGE_SUCCESS;
    
// body_failed:
//     if (__callback)
//         __callback(__buffer, true, status_code::PAYLOAD_TOO_LARGE);
//     return MESSAGE_FAILED;
}


void
message::recv(
    std::istream&      __stream,
    message_callback&& __when_received)
{
    const int k_state_start_line  = 0;
    const int k_state_header_line = 1;
    const int k_state_body        = 2;
    const int k_state_end         = 3;
    
    if (__when_received == nullptr)
        throw impact_error("null callback");
    
    parser_limits limits;
    status_code code = status_code::OK;
    
    message_traits_ptr traits = nullptr;
    header_list headers = {};
    data_recv_callback callback = nullptr;
    unsigned int body_info[2] = { NO_BODY, 0 };
    
    std::string buffer;
    int current_state = k_state_start_line;
    do {
        switch (current_state) {
        
        case k_state_start_line: {
            int result = _M_process_start_line(
                __stream, buffer, limits, traits);
            if (result == MESSAGE_SUCCESS) current_state = k_state_header_line;
            else if (result == VERSION_FAILED)    goto version_failed;
            else /* (result == MESSAGE_FAILED) */ goto message_failed;
        } break;
        
        case k_state_header_line: {
            int result = _M_process_header_line(
                __stream, buffer, limits, body_info, headers);
            if (result == MESSAGE_SUCCESS) {
                __when_received(traits, headers, &callback, code);
                if (body_info[0]) current_state = k_state_body;
                else              current_state = k_state_end;
            }
            else if (result == MESSAGE_FAILED)  goto message_failed;
            else if (result == HEADER_OVERLOAD) goto header_overload;
            /* else (result == MESSAGE_CONTINUE) */
        } break;
        
        case k_state_body: {
            int result = _M_process_body(
                __stream, buffer, limits, body_info, headers, callback);
            if      (result == MESSAGE_SUCCESS ) current_state = k_state_end;
            // do not invoke message callback a second time
            // the error will be reported in the data callback (if it exists)
            else if (result == MESSAGE_FAILED  ) return;
            /* else (result == MESSAGE_CONTINUE) */
        } break;
        
        default: break;
        }
    } while (current_state != k_state_end);

    return;

message_failed:
    code = status_code::BAD_REQUEST;
    goto send_error;
version_failed:
    code = status_code::HTTP_VERSION_NOT_SUPPORTED;
    goto send_error;
header_overload:
    code = status_code::REQUEST_HEADER_FIELDS_TOO_LARGE;
send_error:
    __when_received(traits, headers, &callback, code);
    return;
}


#undef MESSAGE_SUCCESS
#undef MESSAGE_FAILED
#undef VERSION_FAILED
#undef MESSAGE_CONTINUE
#undef NO_BODY
#undef FIXED_BODY
#undef DYNAMIC_BODY


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
