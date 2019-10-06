/**
 * Created by TekuConcept on October 5, 2019
 */

#ifndef _IMPACT_HTTP_MESSAGE_BUILDER_H_
#define _IMPACT_HTTP_MESSAGE_BUILDER_H_

#include <iostream>
#include <string>
#include <memory>

#include "rfc/http/message.h"

namespace impact {
namespace http {

    class message_builder {
    public:
        message_builder();
        ~message_builder();

        void write(const std::string& block);
        void write(
            std::string::const_iterator first,
            std::string::const_iterator last);

        // used for unit-testing
        inline const std::string& buffer() const { return m_buffer_; }
        void clear();

        friend std::istream& operator>>(std::istream&, message_builder&);

    private:
        enum class parsing_state {
            START,
            HEADER,
            BODY
        };

        std::string m_buffer_;
        size_t m_block_start_;
        size_t m_block_end_;
        parsing_state m_state_;
        std::unique_ptr<message_t> m_current_message_;

        bool m_header_ready_;
        bool m_in_empty_line_;

        void _M_process();
        inline void _M_process_start();
        inline void _M_process_header();
        inline void _M_process_body();
    };

}}

#endif
