/**
 * Created by TekuConcept on December 28, 2018
 */

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include "utils/environment.h"
#include "utils/impact_error.h"
#include "utils/string_ops.h"
#include "rfc/http/message.h"

#include <iostream>
#define VERBOSE(x) std::cout << x << std::endl

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

message::message()
: m_has_body_(false), m_is_fixed_body_(false), m_data_buffer_("")
{}


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

message::limits::limits()
{}


void
message::_M_initialize(
    message_traits_ptr*      __traits,
    transfer_encoding_token* __dynamic,
    std::string*             __fixed)
{
    m_traits_ = *__traits;
    
    if (__dynamic && __dynamic->callback) {
        m_is_fixed_body_ = false;
        m_data_          = *__dynamic;
        m_data_buffer_   = "";
        if (m_traits_ && m_traits_->permit_length_header()) {
            m_has_body_  = m_traits_->permit_body();
            m_headers_.push_back(m_data_.header());
        }
        else m_has_body_ = true;
    }
    else if (__fixed) {
        m_is_fixed_body_ = true;
        m_data_buffer_   = *__fixed;
        if (m_traits_ && m_traits_->permit_length_header()) {
            std::ostringstream os;
            os << std::setfill('0') << std::setw(1) << __fixed->size();
            m_has_body_  = m_traits_->permit_body();
            header_token header(field_name::CONTENT_LENGTH, os.str());
            m_headers_.push_back(header);
        }
        else m_has_body_ = true;
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
    if (__message.type() == message_type::NONE)
        throw impact_error("undefined message");
    
    __stream << __message.m_traits_->start_line();
    
    for (const auto& header : __message.m_headers_) {
        __stream << header.name() << ": ";
        __stream << header.value() << "\r\n";
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


inline void
message::_M_process_start_line(parser_context& context)
{
    auto& stream  = *context.stream;
    auto& limits  = context.limits;
    auto& traits  = context.target->m_traits_;
    auto& buffer  = context.buffer;
    size_t result = 0;
    
    buffer.resize(limits.max_line_length, '\0');

    if (!stream.getline(&buffer[0], buffer.size() - 1, '\n'))
        goto message_failed;
    result = stream.gcount();

    if (result <= 0) goto message_failed;
    buffer[result - 1] = '\n'; // getline discards this but it is needed

    try { traits = message_traits::create(buffer.substr(0, result)); }
    catch (...) { goto message_failed; }

    if (traits->http_major() != 1 || traits->http_minor() != 1)
        goto unsupported_version;
    
    context.current_state = recv_state::HEADER_LINE;
    return;
    
message_failed:
    context.code          = status_code::BAD_REQUEST;
    context.current_state = recv_state::DONE;
    return;
unsupported_version:
    context.code          = status_code::HTTP_VERSION_NOT_SUPPORTED;
    context.current_state = recv_state::DONE;
}


inline void
message::_M_process_header_line(parser_context& context)
{
    auto& stream    = *context.stream;
    auto& limits    = context.limits;
    auto& body_info = context.body_info;
    auto& buffer    = context.buffer;
    auto& headers   = context.target->m_headers_;
    size_t result   = 0;

    if (!stream.getline(&buffer[0], buffer.size() - 1, '\n'))
        goto message_failed;
    result = stream.gcount();
    if (result <= 1) goto message_failed;

    if (buffer[0] == '\r') {
        if (body_info[0])
             context.current_state = recv_state::BODY;
        else context.current_state = recv_state::DONE;
        return;
    }
    
    if ((headers.size() >= limits.max_header_limit) ||
        (buffer[result - 2] != '\r'))
        goto header_too_big;
    else buffer[result - 1] = '\n';

    try {
        headers.push_back(header_token(buffer.substr(0, result)));
        auto name = headers.back().name();
        if (name == "Content-Length") {
            if (body_info[0] != NO_BODY)
                goto message_failed;
            body_info[0] = FIXED_BODY;
            body_info[1] = headers.size() - 1;
        }
        else if (name == "Transfer-Encoding") {
            if (body_info[0] != NO_BODY)
                goto message_failed;
            body_info[0] = DYNAMIC_BODY;
            body_info[1] = headers.size() - 1;
        }
    }
    catch (...) { goto message_failed; }

    return;
    
message_failed:
    context.code          = status_code::BAD_REQUEST;
    context.current_state = recv_state::DONE;
    return;
header_too_big:
    context.code          = status_code::REQUEST_HEADER_FIELDS_TOO_LARGE;
    context.current_state = recv_state::DONE;
}


inline void
message::_M_process_fixed_body(parser_context& context)
{
    auto& stream    = *context.stream;
    auto& limits    = context.limits;
    auto& body_info = context.body_info;
    auto& buffer    = context.buffer;
    auto& headers   = context.target->m_headers_;
    
    if (body_info[2] == 0) { // initial pass
        unsigned int header_index = body_info[1];
        std::string string_length = headers[header_index].value();
        unsigned int body_size;
        
        try { body_size = std::stoul(string_length, 0, 10); }
        catch (std::out_of_range&) { goto body_too_big; }
        catch (...) { goto message_failed; }
        
        if (body_size > limits.payload_size_limit)
            goto body_too_big;
        buffer.resize(body_size);
        body_info[2] = body_size;
    }
    else {
        unsigned int start_index = body_info[3];
        unsigned int length = buffer.size() - start_index;
        if (!stream.read(&buffer[start_index], length))
            goto message_failed;
        body_info[3] += stream.gcount();
        
        if (body_info[3] == body_info[2]) {
            context.target->m_data_buffer_ = std::move(buffer);
            context.target->m_has_body_    = true;
            context.current_state = recv_state::DONE;
        }
    }
    
    return;
    
message_failed:
    context.code          = status_code::BAD_REQUEST;
    context.current_state = recv_state::DONE;
    return;
body_too_big:
    context.code          = status_code::PAYLOAD_TOO_LARGE;
    context.current_state = recv_state::DONE;
}


inline void
message::_M_process_dynamic_init(parser_context& context)
{
    VERBOSE("_M_process_dynamic_init");
    auto& body_info = context.body_info;
    auto& encodings = context.encodings;
    
    std::regex delimiter(",[\t ]*"); // ABNF #rule
    auto tokens = impact::internal::split(
        &context.target->m_headers_[body_info[1]].value(),
        delimiter
    );

    bool has_chunked = false;
    case_string case_token;

    for (auto& token : tokens) {
        impact::swap(case_token, token);
        if (has_chunked) goto message_failed;
        else if (case_token == "chunked")
            has_chunked = true;
        encodings.push_back(transfer_encoding::get_by_name(case_token));
        if (encodings.back() == nullptr) goto not_implemented;
    }

    if (has_chunked) body_info[4] = 1; // goto chunk size
    else             body_info[4] = 4; // goto unknown payload
    context.target->m_data_buffer_.clear();

    return;

message_failed:
    context.code          = status_code::BAD_REQUEST;
    context.current_state = recv_state::DONE;
    return;
not_implemented:
    context.code          = status_code::NOT_IMPLEMENTED;
    context.current_state = recv_state::DONE;
    return;
}


inline void
message::_M_process_chunk_size(parser_context& context)
{
    VERBOSE("_M_process_chunk_size");
    auto& stream    = *context.stream;
    auto& limits    = context.limits;
    auto& buffer    = context.buffer;
    auto& body_info = context.body_info;
    size_t result   = 0;
    
    if (!stream.getline(&buffer[0], buffer.size() - 1, '\n'))
        goto message_failed;
    result = stream.gcount();
    if (result <= 1) goto message_failed;

    if (buffer[result - 2] != '\r')
        goto message_failed;
    else buffer[result - 1] = '\n';
    
    size_t chunk_size;
    // optimize: use iterators, or use char* and length
    // enhancement: chunk_size = npos when size_t overflow
    if (!chunked_encoding::decode_first_line(
        buffer.substr(0, result), &chunk_size, NULL))
        goto message_failed;
    
    if (chunk_size == 0) {
        body_info[4] = 3; // goto trailer part
        body_info[2] = 1; // set next decoding index
        context.target->m_has_body_ = true;
        context.target->m_headers_[body_info[1]] = header_token(
            field_name::CONTENT_LENGTH, std::to_string(body_info[2]));
    }
    else if ((body_info[2] + chunk_size) > limits.payload_size_limit)
        goto body_too_big;
    else {
        body_info[2] += chunk_size;
        body_info[4] = 2; // goto payload
        context.target->m_data_buffer_.resize(body_info[2]);
    }
    
    return;
    
message_failed:
    context.code          = status_code::BAD_REQUEST;
    context.current_state = recv_state::DONE;
    return;
body_too_big:
    context.code          = status_code::PAYLOAD_TOO_LARGE;
    context.current_state = recv_state::DONE;
}


inline void
message::_M_process_chunk_payload(parser_context& context)
{
    VERBOSE("_M_process_chunk_payload");
    auto& stream    = *context.stream;
    auto& body_info = context.body_info;
    auto& buffer    = context.target->m_data_buffer_;
    size_t result   = 0;
    
    unsigned int start_index = body_info[3];
    unsigned int length = buffer.size() - start_index;
    if (!stream.read(&buffer[start_index], length))
        goto message_failed;
    body_info[3] += stream.gcount();
    
    if (body_info[3] == body_info[2]) {
        // read body CRLF
        if (!stream.getline(&context.buffer[0], buffer.size() - 1, '\n'))
            goto message_failed;
        result = stream.gcount();
        if (result != 2 || context.buffer[0] != '\r')
            goto message_failed;
        context.body_info[4] = 1; // goto next chunk size
    }
    
    return;
    
message_failed:
    context.code          = status_code::BAD_REQUEST;
    context.current_state = recv_state::DONE;
}


inline void
message::_M_process_chunk_trailer(parser_context& context)
{
    VERBOSE("_M_process_chunk_trailer");
    // NOTE: Copy-Paste of _M_process_header_line w/ state tweaks
    auto& stream    = *context.stream;
    auto& limits    = context.limits;
    auto& body_info = context.body_info;
    auto& buffer    = context.buffer;
    auto& headers   = context.target->m_headers_;
    size_t result   = 0;

    if (!stream.getline(&buffer[0], buffer.size() - 1, '\n'))
        goto message_failed;
    result = stream.gcount();
    if (result <= 1) goto message_failed;

    if (buffer[0] == '\r') {
        body_info[4] = 5; // goto decode
        return;
    }
    
    if ((headers.size() >= limits.max_header_limit) ||
        (buffer[result - 2] != '\r'))
        goto header_too_big;
    else buffer[result - 1] = '\n';

    try {
        header_token header(buffer.substr(0, result));
        auto name = header.name();
        if ((name != "Content-Length") &&
            (name != "Transfer-Encoding"))
            headers.push_back(header);
    }
    catch (...) { goto message_failed; }

    return;
    
message_failed:
    context.code          = status_code::BAD_REQUEST;
    context.current_state = recv_state::DONE;
    return;
header_too_big:
    context.code          = status_code::REQUEST_HEADER_FIELDS_TOO_LARGE;
    context.current_state = recv_state::DONE;
}


inline void
message::_M_process_unknown_payload(parser_context& context)
{
    VERBOSE("_M_process_unknown_payload");
    auto& stream    = *context.stream;
    auto& body_info = context.body_info;
    auto& buffer    = context.buffer;
    auto& limits    = context.limits;
    size_t result   = 0;
    
    unsigned int start_index = body_info[3];
    if (!stream.read(&buffer[start_index], buffer.size()))
        goto message_failed;
    result = stream.gcount();
    body_info[3] += result;
    if (body_info[3] > limits.payload_size_limit)
        goto message_failed;

    context.target->m_data_buffer_.append(
        buffer.begin(), buffer.begin() + result);

    if (stream.eof()) {
        context.target->m_has_body_ = true;
        body_info[4] = 5; // goto decode state
        body_info[2] = 0; // reset decoding index
    }
    
    return;
    
message_failed:
    context.code          = status_code::BAD_REQUEST;
    context.current_state = recv_state::DONE;
}


inline void
message::_M_process_decode(parser_context& context)
{
    auto& encodings = context.encodings;
    auto& body_info = context.body_info;
    
    if (body_info[2] == context.encodings.size()) {
        body_info[4] = 6;
        context.current_state = recv_state::DONE;
    }
    else {
        auto& codec = encodings[context.encodings.size() - 1 - body_info[2]];
        body_info[2]++;
        context.target->m_data_buffer_ =
            codec->decode(context.target->m_data_buffer_);
    }
}


inline void
message::_M_process_dynamic_body(parser_context& context)
{
    VERBOSE("_M_process_dynamic_body");
    
    switch (context.body_info[4]) {
    case 0  /* init            */: _M_process_dynamic_init(context);    break;
    case 1  /* chunk size      */: _M_process_chunk_size(context);      break;
    case 2  /* chunk payload   */: _M_process_chunk_payload(context);   break;
    case 3  /* chunk trailer   */: _M_process_chunk_trailer(context);   break;
    case 4  /* unknown payload */: _M_process_unknown_payload(context); break;
    case 5  /* decode          */: _M_process_decode(context);          break;
    default /* done            */:
        context.current_state = recv_state::DONE; break;
    }
}


inline void
message::_M_process_body(parser_context& context)
{
    auto& body_info = context.body_info;
    context.target->m_is_fixed_body_ = true;
    context.target->m_has_body_ = context.target->m_traits_->permit_body();

    /*
    Do not proceed if no body is expected. Doing so may
    may result in parsing the next message as a body. If
    a body does exists when it is not expected, it will
    be treated as an invalid message when parsed.
    */
    if (!context.target->m_has_body_) return;
    
    if (body_info[0] == FIXED_BODY)
        _M_process_fixed_body(context);
    else /* dynamic */
        _M_process_dynamic_body(context);
    
    return;
}


status_code
message::recv(
    std::istream& __stream,
    message*      __message,
    limits        __limits)
{
    if (__message == NULL) return status_code::OK;
    
    parser_context context;
    context.limits        = __limits;
    context.stream        = &__stream;
    /* type, header, expected_buf_size, actual_buf_size, meta */
    context.body_info     = { NO_BODY, 0, 0, 0, 0 };
    context.buffer        = "";
    context.target        = __message;
    context.code          = status_code::OK;
    context.current_state = recv_state::START_LINE;
    
    __message->m_traits_        = nullptr;
    __message->m_headers_       = {};
    __message->m_has_body_      = false;
    __message->m_is_fixed_body_ = false;
    __message->m_data_          = transfer_encoding_token();
    __message->m_data_buffer_   = "";
    
    do {
        switch (context.current_state) {
        case recv_state::START_LINE:  _M_process_start_line(context);  break;
        case recv_state::HEADER_LINE: _M_process_header_line(context); break;
        case recv_state::BODY:        _M_process_body(context);        break;
        default: break;
        }
    } while (context.current_state != recv_state::DONE);

    return context.code;
}


message_type
message::type() const noexcept
{
    if (m_traits_) return m_traits_->type();
    else return message_type::NONE;
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
        if (header.name() == "Transfer-Encoding" ||
            header.name() == "Content-Length") {
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


const std::string&
message::body() noexcept
{
    if (!m_is_fixed_body_) m_data_buffer_.clear();
    return m_data_buffer_;
}


void
message::body(std::string __data)
{ _M_initialize(&m_traits_, NULL, &__data); }


void
message::body(
    transfer_encoding_token::list       __encodings,
    std::function<void(std::string*)>&& __data_callback)
{
    transfer_encoding_token token(__encodings, std::move(__data_callback));
    _M_initialize(&m_traits_, &token, NULL);
}


void
message::body(transfer_encoding_token __data)
{ _M_initialize(&m_traits_, &__data, NULL); }


message_traits_ptr
message::traits() const noexcept
{
    return m_traits_;
}


void
message::traits(message_traits_ptr __traits)
{
    m_traits_ = __traits;
}
