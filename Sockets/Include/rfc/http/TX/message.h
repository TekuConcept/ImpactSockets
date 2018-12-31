/**
 * Created by TekuConcept on December 28, 2018
 */

#ifndef _IMPACT_HTTP_MESSAGE_H_
#define _IMPACT_HTTP_MESSAGE_H_

#include <string>
#include <vector>
#include <functional>
#include "utils/case_string.h"
#include "rfc/http/header_token.h"
#include "rfc/http/TX/transfer_encoding.h"
#include "rfc/http/message_traits.h"
#include "rfc/http/types.h"

namespace impact {
namespace http {
    // caching object
    // (speed up execution by reducing validation checks)
    class transfer_encoding_token {
    public:
        typedef std::vector<transfer_encoding_ptr> list;
        
        transfer_encoding_token(list encodings,
            std::function<void(std::string*)> callback);
        ~transfer_encoding_token();
        
        std::function<void(std::string*)> callback;
    
    private:
        header_token m_header_;
        list m_transfer_encodings_;
        
        transfer_encoding_token();
    
    public:
        inline const list& encodings()
        { return m_transfer_encodings_; }
        inline const header_token& header() { return m_header_; }
        
        friend class message;
    };
    

    class message {
    public:
        message(message_traits_ptr);
        message(message_traits_ptr, transfer_encoding_token data);
        message(message_traits_ptr, std::string data);
        
        /* [- request message convinience ctors -] */
        message(std::string method, std::string target);
        message(std::string method, std::string target,
            transfer_encoding_token::list encodings,
            std::function<void(std::string*)> data_callback);
        message(method_token method, target_token target,
            transfer_encoding_token data);
        message(std::string method, std::string target, std::string data);
        message(method_token method, target_token target, std::string data);
        
        /* [- response message convinience ctors -] */
        message(int status_code, std::string reason_phrase);
        message(int status_code, std::string reason_phrase,
            transfer_encoding_token::list encodings,
            std::function<void(std::string*)> data_callback);
        message(int status_code, std::string reason_phrase,
            transfer_encoding_token data);
        message(int status_code, std::string reason_phrase, std::string data);
        
        virtual ~message();
        
        void send(std::ostream& stream);
        message_type type() const noexcept;
        
        const std::vector<header_token>& headers() const noexcept;
        void headers(std::vector<header_token> list);
        
    private:
        std::vector<header_token> m_headers_;
        message_traits_ptr        m_traits_;
        
        bool                    m_has_body_;
        bool                    m_is_fixed_body_;
        transfer_encoding_token m_data_;
        std::string             m_data_buffer_;
        
        void _M_initialize(message_traits_ptr*,
            transfer_encoding_token*,std::string*);
    };
}}

#endif
