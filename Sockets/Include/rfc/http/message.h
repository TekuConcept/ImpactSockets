/**
 * Created by TekuConcept on December 28, 2018
 */

#ifndef _IMPACT_HTTP_MESSAGE_H_
#define _IMPACT_HTTP_MESSAGE_H_

#include <string>
#include <vector>
#include <array>
#include <functional>
#include "utils/case_string.h"
#include "rfc/http/header_token.h"
#include "rfc/http/transfer_encoding.h"
#include "rfc/http/message_traits.h"
#include "rfc/http/types.h"

namespace impact {
namespace http {
    class transfer_encoding_token {
    public:
        typedef std::vector<transfer_encoding_ptr> list;
        
        transfer_encoding_token(list encodings,
            std::function<void(std::string*)>&& callback);
        ~transfer_encoding_token();
        
        std::function<void(std::string*)> callback;
    
    private:
        header_token m_header_;
        list m_transfer_encodings_;
        
        transfer_encoding_token();
    
    public:
        inline const list& encodings() const noexcept
        { return m_transfer_encodings_; }
        inline const header_token& header() const noexcept
        { return m_header_; }
        
        friend class message;
    };
    

    class message {
    public:
        typedef std::vector<header_token> header_list;
        typedef std::function<void(const char*,unsigned int,bool,status_code)>
            data_recv_callback;
        typedef std::function<void(const message_traits_ptr&,const header_list&,
            data_recv_callback*,enum status_code)> message_callback;
        
        struct limits {
            unsigned int max_line_length    = 8000;    // 8 kB
            unsigned int max_header_limit   = 50;
            unsigned int payload_size_limit = 1000000; // 1 MB
            limits();
        };
        
        /* [- generic ctors -] */
        message();
        message(message_traits_ptr);
        message(message_traits_ptr, transfer_encoding_token data);
        message(message_traits_ptr, std::string data);
        
        /* [- request message convinience ctors -] */
        message(std::string method, std::string target);
        message(std::string method, std::string target,
            transfer_encoding_token::list encodings,
            std::function<void(std::string*)>&& data_callback);
        message(method_token method, target_token target,
            transfer_encoding_token data);
        message(std::string method, std::string target, std::string data);
        message(method_token method, target_token target, std::string data);
        
        /* [- response message convinience ctors -] */
        message(int status_code, std::string reason_phrase);
        message(int status_code, std::string reason_phrase,
            transfer_encoding_token::list encodings,
            std::function<void(std::string*)>&& data_callback);
        message(int status_code, std::string reason_phrase,
            transfer_encoding_token data);
        message(int status_code, std::string reason_phrase, std::string data);
        
        
        virtual ~message();
        
        static void send(std::ostream& stream, const message& message);
        static status_code recv(std::istream& stream, message* message,
            limits parser_limits = limits());
        
        message_type type() const noexcept;
        
        const std::vector<header_token>& headers() const noexcept;
        void headers(std::vector<header_token> list);
        
        const std::string& body() noexcept;
        void body(std::string data);
        void body(transfer_encoding_token::list encodings,
            std::function<void(std::string*)>&& data_callback);
        void body(transfer_encoding_token data);
        
        message_traits_ptr traits() const noexcept;
        void traits(message_traits_ptr traits);
        
    private:
        std::vector<header_token> m_headers_;
        message_traits_ptr        m_traits_;
        bool                      m_has_body_;
        bool                      m_is_fixed_body_;
        transfer_encoding_token   m_data_;
        std::string               m_data_buffer_;
        
        void _M_initialize(message_traits_ptr*,
            transfer_encoding_token*,std::string*);
        
        enum class recv_state { START_LINE, HEADER_LINE, BODY, DONE };
        enum body_type { NO_BODY = 0, FIXED_BODY, DYNAMIC_BODY };
        struct parser_context {
            struct limits              limits;
            std::istream*              stream;
            std::array<unsigned int,5> body_info;
            std::string                buffer;
            message*                   target;
            enum status_code           code;
            enum recv_state            current_state;
            std::vector<transfer_encoding_ptr> encodings;
        };
        
        static inline void _M_process_start_line(parser_context&);
        static inline void _M_process_header_line(parser_context&);
        static inline void _M_process_fixed_body(parser_context&);
        static inline void _M_process_dynamic_body(parser_context&);
        static inline void _M_process_dynamic_init(parser_context&);
        static inline void _M_process_chunk_size(parser_context&);
        static inline void _M_process_chunk_payload(parser_context&);
        static inline void _M_process_chunk_trailer(parser_context&);
        static inline void _M_process_unknown_payload(parser_context&);
        static inline void _M_process_decode(parser_context&);
        static inline void _M_process_body(parser_context&);
    };
}}

#endif
