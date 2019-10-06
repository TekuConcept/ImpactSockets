/**
 * Created by TekuConcept on October 5, 2019
 */

#include "rfc/http/message_builder.h"

using namespace impact;
using namespace http;

#define V(x) std::cout << x << std::endl;

/**
 * TODO
 * - start-line
 * - *(header-line)
 * - empty-line
 * - body
 */


message_builder::message_builder()
: m_header_ready_(false),
  m_in_empty_line_(false)
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
    m_block_start_ = 0;
    m_block_end_   = 0;
    m_state_       = parsing_state::START;
    m_buffer_.clear();
}


void
message_builder::_M_process()
{
    while (m_block_end_ < m_buffer_.size()) {
        switch (m_state_) {
        case parsing_state::START:  _M_process_start();  break;
        case parsing_state::HEADER: _M_process_header(); break;
        case parsing_state::BODY:   _M_process_body();   break;
        }
    }
}


inline void
message_builder::_M_process_start()
{
    // TODO: limit search to N characters
    for (; m_block_end_ < m_buffer_.size(); m_block_end_++) {
        if (m_buffer_[m_block_end_] == '\n') {
            try {
                V(m_buffer_.substr(0, m_block_end_ + 1));
                m_buffer_ = m_buffer_.substr(m_block_end_ + 1, m_buffer_.npos);
                m_block_end_ = 0;
                // auto traits = message_traits::create(
                //     m_buffer_.substr(0, m_block_end_));
                // m_current_message_.reset(new message_t
                //     (message_t(std::move(traits))));
                m_state_ = parsing_state::HEADER;
                break;
            }
            catch (...) {
                // on_message_failed()
            }
        }
    }
}


inline void
message_builder::_M_process_header()
{
    // TODO: limit search to N characters
    // TODO: limit to max of M headers
    for (; m_block_end_ < m_buffer_.size(); m_block_end_++) {
        if (m_in_empty_line_) {
            if (m_buffer_[m_block_end_] == '\n') {
                // empty-line reached
                m_buffer_    = m_buffer_.substr(2, m_buffer_.npos);
                m_block_end_ = 0;
                // TODO: determine from headers and message traits
                // if next parsing state is BODY or START
                m_state_     = parsing_state::BODY;
                break;
            }
            else {
                // on_message_failed()
            }
            m_in_empty_line_ = false;
        }
        if (m_header_ready_) {
            m_header_ready_ = false;
            if (m_buffer_[m_block_end_] == '\r')
                m_in_empty_line_ = true; // possible empty-line
            else if (m_buffer_[m_block_end_] == ' ')
                continue; // obsolete line folding
            // else possible new header
            try {
                V(m_buffer_.substr(0, m_block_end_));
                m_buffer_    = m_buffer_.substr(m_block_end_, m_buffer_.npos);
                m_block_end_ = 0;
                // parse header
                // insert header into message
                // reserve a copy if header describes the body
            }
            catch (...) {
                // on_message_failed()
            }
        }
        else if (m_buffer_[m_block_end_] == '\n')
            m_header_ready_ = true;
    }
}


inline void
message_builder::_M_process_body()
{
    // TODO
    m_state_ = parsing_state::START;
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
