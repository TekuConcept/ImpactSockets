/**
 * Created by TekuConcept on October 5, 2019
 */

#ifndef _IMPACT_HTTP_MESSAGE_BUILDER_H_
#define _IMPACT_HTTP_MESSAGE_BUILDER_H_

#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "rfc/http/message.h"
#include "rfc/http/transfer_coding.h"

namespace impact {
namespace http {

    class message_builder_observer {
    public:
        enum class error_id {
            START_LINE_PARSER_ERROR,
            HEADER_LINE_PARSER_ERROR,
            BODY_PARSER_ERROR,
            LINE_LENGTH_EXCEEDED,
            HEADER_LIMIT_EXCEEDED,
            CHUNK_LIMIT_EXCEEDED,
            DUPLICATE_BODY_HEADERS
        };

        struct payload_fragment {
            std::string::const_iterator data_begin;
            std::string::const_iterator data_end;
            bool eop;
            bool continuous;
            std::vector<chunked_coding::extension_t> chunk_extensions;
            header_list trailers;
        };

        virtual ~message_builder_observer() = default;
        virtual void on_error(error_id error) = 0;
        virtual void on_message(std::unique_ptr<message_t> message) = 0;
        virtual void on_data(const payload_fragment& fragment) = 0;
    };


    class message_builder {
    public:
        message_builder();
        virtual ~message_builder();

        void write(const std::string& block);
        void write(
            std::string::const_iterator first,
            std::string::const_iterator last);

        void clear();

        inline void register_observer(message_builder_observer* observer)
        { m_observer_ = observer; }

        friend std::istream& operator>>(std::istream&, message_builder&);

    private:
        enum class parsing_state {
            ERROR,
            START,
            HEADER,
            BODY
        };

        enum class body_format {
            UNKNOWN,
            CONTENT_LENGTH,
            CHUNKED_CODING,
            CONTINUOUS
        };

        std::string m_buffer_;
        size_t m_block_end_;
        parsing_state m_state_;
        std::unique_ptr<message_t> m_current_message_;
        std::unique_ptr<header_t> m_body_header_;
        message_builder_observer* m_observer_;

        int m_header_state_;
        body_format m_body_format_;
        int m_chunk_state_;
        std::vector<chunked_coding::extension_t> m_chunk_ext_;
        header_list m_chunk_trailers_;
        size_t m_body_length_;
        bool m_is_response_;

        bool m_header_ready_;
        bool m_in_empty_line_;

        void _M_process();
        inline void _M_process_start();
        inline void _M_process_header();
        inline bool _M_determine_body_format();
        inline void _M_process_body();
        inline void _M_parse_chunk();
        void _M_process_empty_line();
        bool _M_insert_header(header_list&, bool = false);
        void _M_process_empty_chunk_line();

    protected:
        // for unit testing
        inline const std::string& buffer() const { return m_buffer_; }
        inline parsing_state state() { return m_state_; }
    };

}}

#endif
