/**
 * Created by TekuConcept on October 5, 2019
 */

#include "rfc/http/message_builder.h"

#include <algorithm>

#include "utils/abnf_ops.h"
#include "rfc/http/abnf_ops.h"

using namespace impact;
using namespace http;

#define V(x) std::cout << x << std::endl;

#define EMIT_ERROR_FLAG(error, flag) \
m_state_ = parsing_state::ERROR; \
if (m_observer_) \
    m_observer_->on_error(error); \
return flag;

#define EMIT_ERROR(error) \
m_state_ = parsing_state::ERROR; \
if (m_observer_) \
    m_observer_->on_error(error); \
return;


inline void
print_char(char c)
{
    if (c == '\r')
        std::cout << "\\r";
    else if (c == '\n')
        std::cout << "\\n\n";
    else std::cout << c;
}


message_builder::message_builder()
: m_observer_(NULL)
{ clear(); }


message_builder::~message_builder() = default;


void
message_builder::write(const std::string& __block)
{
    m_buffer_.append(__block);
    _M_process();
}


void
message_builder::write(
    std::string::const_iterator __first,
    std::string::const_iterator __last)
{
    m_buffer_.append(__first, __last);
    _M_process();
}


void
message_builder::clear()
{
    m_buffer_.clear();
    m_block_end_       = 0;
    m_state_           = parsing_state::START;
    m_current_message_ = nullptr;
    m_body_header_     = nullptr;
    m_header_ready_    = false;
    m_in_empty_line_   = false;
}


void
message_builder::_M_process()
{
    while ((m_block_end_ < m_buffer_.size()) &&
        (m_state_ != parsing_state::ERROR)) {
        switch (m_state_) {
        case parsing_state::START:  _M_process_start();  break;
        case parsing_state::HEADER: _M_process_header(); break;
        case parsing_state::BODY:   _M_process_body();   break;
        case parsing_state::ERROR:                       break;
        }
    }
}


inline void
message_builder::_M_process_start()
{
    using error_id = message_builder_observer::error_id;
    for (; m_block_end_ < m_buffer_.size(); m_block_end_++) {
        print_char(m_buffer_[m_block_end_]);
        if (m_block_end_ > message_t::message_limits().max_line_length)
        {
            V(__FUNCTION__ << "() 103");
            EMIT_ERROR(error_id::LINE_LENGTH_EXCEEDED)
        }
        if (m_buffer_[m_block_end_] == '\n') {
            try {
                auto traits = message_traits::create(
                    m_buffer_.substr(0, m_block_end_ + 1));
                m_current_message_.reset(new message_t(std::move(traits)));
                m_buffer_ = m_buffer_.substr(m_block_end_ + 1, m_buffer_.npos);
                m_block_end_ = 0;
                m_state_ = parsing_state::HEADER;
                m_is_response_ = m_current_message_->traits()->type() ==
                    message_traits::message_type::RESPONSE;
                m_header_state_ = 0;
                break;
            }
            catch (...) {
                V(__FUNCTION__ << "() 120");
                EMIT_ERROR(error_id::START_LINE_PARSER_ERROR)
            }
        }
    }
}


inline void
message_builder::_M_process_header()
{
    using error_id = message_builder_observer::error_id;
    for (; m_block_end_ < m_buffer_.size(); m_block_end_++) {
        char c = m_buffer_[m_block_end_];

        print_char(c);
        if (m_block_end_ > (message_t::message_limits().max_line_length + 1)) {
            V(__FUNCTION__ << "() 133 " << m_block_end_ << " : "
                << (message_t::message_limits().max_line_length + 1));
            EMIT_ERROR(error_id::LINE_LENGTH_EXCEEDED)
        }

        switch (m_header_state_) {
        case 0:
            if (c == '\r') m_header_state_ = 1; // empty line
            else if (internal::TCHAR(c)) m_header_state_ = 2; // header line
            else { EMIT_ERROR(error_id::HEADER_LINE_PARSER_ERROR) }
            break;

        case 1:
            _M_process_empty_line();
            return;

        case 2:
            if (m_in_empty_line_) {
                _M_process_empty_line();
                return;
            }
            else if (m_header_ready_)
            { if (!_M_insert_header(m_current_message_->headers())) return; }
            else if (m_buffer_[m_block_end_] == '\n')
                m_header_ready_ = true;
            break;
        }
    }
}


void
message_builder::_M_process_empty_line()
{
    using error_id = message_builder_observer::error_id;
    if (m_buffer_[m_block_end_] == '\n') {
        // empty-line reached
        m_buffer_    = m_buffer_.substr(2, m_buffer_.npos);
        m_block_end_ = 0;
        if (!m_current_message_->traits()->permit_body() ||
            m_body_header_ == nullptr) {
            m_state_ = parsing_state::START;
            m_body_header_ = nullptr;
        }
        else {
            m_state_ = parsing_state::BODY;
            if (!_M_determine_body_format()) return;
        }
        if (m_observer_)
            m_observer_->on_message(std::move(m_current_message_));
    }
    else {
        m_state_ = parsing_state::ERROR;
        if (m_observer_)
            m_observer_->on_error(error_id::HEADER_LINE_PARSER_ERROR);
    }
    m_in_empty_line_ = false;
}


inline bool
message_builder::_M_determine_body_format()
{
    using error_id = message_builder_observer::error_id;
    const auto& value = m_body_header_->value();
    if (m_body_header_->m_field_name_id_ == field_name::CONTENT_LENGTH) {
        m_body_format_ = body_format::CONTENT_LENGTH;
        for (auto c : value) {
            if (!impact::internal::DIGIT(c)) {
                m_state_ = parsing_state::ERROR;
                if (m_observer_)
                    m_observer_->on_error(error_id::HEADER_LINE_PARSER_ERROR);
                return false;
            }
        }
        try { m_body_length_ = std::stoi(value); }
        catch (...) {
            m_state_ = parsing_state::ERROR;
            if (m_observer_)
                m_observer_->on_error(error_id::HEADER_LINE_PARSER_ERROR);
            return false;
        }
    }
    else {
        m_body_format_ = body_format::UNKNOWN;
        m_chunk_state_ = 0;
        m_chunk_ext_.clear();
        m_chunk_trailers_.clear();
        if (value.size() >= 7) {
            auto offset = value.size() - 7;
            m_body_format_ = (
                (value[offset + 0] == 'c' || value[offset + 0] == 'C') ||
                (value[offset + 1] == 'h' || value[offset + 1] == 'H') ||
                (value[offset + 2] == 'u' || value[offset + 2] == 'U') ||
                (value[offset + 3] == 'n' || value[offset + 3] == 'N') ||
                (value[offset + 4] == 'k' || value[offset + 4] == 'K') ||
                (value[offset + 5] == 'e' || value[offset + 5] == 'E') ||
                (value[offset + 6] == 'd' || value[offset + 6] == 'D')
            ) ? body_format::CHUNKED_CODING : body_format::CONTINUOUS;
        }
        else if (m_is_response_) m_body_format_ = body_format::CONTINUOUS;

        if ((!m_is_response_ && m_body_format_ == body_format::CONTINUOUS) ||
            (m_body_format_ == body_format::UNKNOWN)) {
            m_state_ = parsing_state::ERROR;
            if (m_observer_)
                m_observer_->on_error(error_id::HEADER_LINE_PARSER_ERROR);
            return false;
        }
    }
    m_body_header_ = nullptr; // no longer needed, reseting for next message
    return true;
}


bool
message_builder::_M_insert_header(header_list& __headers)
{
    using error_id = message_builder_observer::error_id;
    m_header_ready_ = false;
    // check next char (non-inclusive)
    if (m_buffer_[m_block_end_] == '\r')
        m_in_empty_line_ = true; // possibly empty-line
    else if (m_buffer_[m_block_end_] == ' ')
        return true; // obsolete line folding
    // else possibly new header
    try {
        if (__headers.size() >=
            message_t::limits().max_header_limit)
        { EMIT_ERROR_FLAG(error_id::HEADER_LIMIT_EXCEEDED, false) }
        __headers.push_back(
            header_t(m_buffer_.substr(0, m_block_end_)));
        if (__headers.back().m_describes_body_size_) {
            if (m_body_header_ != nullptr)
            { EMIT_ERROR_FLAG(error_id::DUPLICATE_BODY_HEADERS, false) }
            else m_body_header_.reset(new header_t(
                __headers.back()));
        }
        m_buffer_    = m_buffer_.substr(m_block_end_, m_buffer_.npos);
        m_block_end_ = 0;
    }
    catch (...)
    { EMIT_ERROR_FLAG(error_id::HEADER_LINE_PARSER_ERROR, false) }
    return true;
}


inline void
message_builder::_M_process_body()
{
    switch (m_body_format_) {

    case body_format::CONTENT_LENGTH: {
        auto count = std::min(m_body_length_, m_buffer_.size());
        if (m_observer_) {
            message_builder_observer::payload_fragment fragment;
            fragment.data_begin = m_buffer_.begin();
            fragment.data_end   = m_buffer_.begin() + count;
            fragment.eop        = m_buffer_.size() >= m_body_length_;
            fragment.continuous = false;
            m_observer_->on_data(fragment);
        }
        m_body_length_ -= count;
        m_block_end_    = 0;
        m_buffer_       = m_buffer_.substr(count, m_buffer_.npos);
        if (m_body_length_ == 0)
            m_state_ = parsing_state::START;
    } break;

    case body_format::CHUNKED_CODING: _M_parse_chunk(); break;

    default: /* continuous */ {
        if (m_observer_) {
            message_builder_observer::payload_fragment fragment;
            fragment.data_begin = m_buffer_.begin();
            fragment.data_end   = m_buffer_.end();
            fragment.eop        = false;
            fragment.continuous = true;
            m_observer_->on_data(fragment);
        }
        m_buffer_.clear();
    } break;
    }
}


inline void
message_builder::_M_parse_chunk()
{
    using error_id = message_builder_observer::error_id;
    for (; m_block_end_ < m_buffer_.size(); m_block_end_++) {
        switch (m_chunk_state_) {
        case 0: // chunk header
            if (m_block_end_ > (message_t::message_limits().max_line_length - 1))
            { EMIT_ERROR(error_id::LINE_LENGTH_EXCEEDED) }
            if (m_buffer_[m_block_end_] == '\r')
                m_chunk_state_ = 1; // CRLF 1
            break;

        case 1: // CRLF 1
            if (m_buffer_[m_block_end_] != '\n')
            { EMIT_ERROR(error_id::BODY_PARSER_ERROR) }
            else {
                // parse chunk line
                //     - parse chunk size (m_body_length_ = parse())
                //     - varify chunk size within limits
                //     - parse chunk extensions
                m_chunk_state_ = m_body_length_ > 0 ? 2 : 5;
                m_block_end_ = -1; // loop increments this to 0
            }
            break;

        case 2: { // chunk body
            auto count = std::min(m_body_length_, m_buffer_.size());
            m_body_length_ -= count;
            m_block_end_   += count - 1;
            if (m_body_length_ == 0)
                m_chunk_state_ = 3; // CR
        } break;


        case 3: // CR
            if (m_buffer_[m_block_end_] != '\r')
            { EMIT_ERROR(error_id::BODY_PARSER_ERROR) }
            else m_chunk_state_ = 4; // LF
            break;
        
        case 4: // LF
            if (m_buffer_[m_block_end_] != '\n')
            { EMIT_ERROR(error_id::BODY_PARSER_ERROR) }
            else {
                message_builder_observer::payload_fragment fragment;
                fragment.continuous       = false;
                fragment.eop              = false;
                fragment.chunk_extensions = std::move(m_chunk_ext_);
                fragment.data_begin       = m_buffer_.begin();
                fragment.data_end         = m_buffer_.begin() + m_block_end_;
                if (m_observer_)
                    m_observer_->on_data(fragment);
                m_state_ = parsing_state::START;
                m_block_end_ = 0;
                return;
            }
            break;
        
        case 5: // end of payload
            if (m_block_end_ > (message_t::message_limits().max_line_length + 1))
            { EMIT_ERROR(error_id::LINE_LENGTH_EXCEEDED) }
            if (m_in_empty_line_) {
                if (m_buffer_[m_block_end_] == '\r') {

                }
                else { EMIT_ERROR(error_id::BODY_PARSER_ERROR) }
                return;
            }
            else if (m_header_ready_)
            { if (!_M_insert_header(m_chunk_trailers_)) return; }
            else if (m_buffer_[m_block_end_] == '\n')
                m_header_ready_ = true;
            break;
        }
    }
}


namespace impact {
namespace http {

    std::istream&
    operator>>(
        std::istream&    __is,
        message_builder& __builder)
    {
        std::string block(128, '\0');
        size_t length;
        do {
            length = __is.readsome(&block[0], block.size());
            if (length > 0)
                __builder.write(block.begin(), block.begin() + length);
        } while (length > 0);
        return __is;
    }

}}
