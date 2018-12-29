/**
 * Created by TekuConcept on December 28, 2018
 */

#ifndef _IMPACT_HTTP_MESSAGE_H_
#define _IMPACT_HTTP_MESSAGE_H_

#include <string>
#include <vector>
#include <functional>
#include "utils/case_string.h"
#include "rfc/http/TX/header_token.h"
#include "rfc/http/TX/transfer_encoding.h"

namespace impact {
namespace http {
    // caching object
    // (speed up execution by reducing validation checks)
    class transfer_encoding_token {
    public:
        typedef std::shared_ptr<transfer_encoding> transfer_encoding_ptr;
        typedef std::vector<transfer_encoding_ptr> transfer_encoding_list;
        
        transfer_encoding_token(transfer_encoding_list encodings,
            std::function<void(std::string*)> callback = nullptr);
        ~transfer_encoding_token();
        
        std::function<void(std::string*)> callback;
    
    private:
        header_token m_header_;
        transfer_encoding_list m_transfer_encodings_;
    
    public:
        inline const transfer_encoding_list& encodings()
        { return m_transfer_encodings_; }
        inline const header_token& header() { return m_header_; }
    };

    
    enum class message_type { REQUEST, RESPONSE };
    
    
    class message {
    public:
        virtual ~message();
        
        void send(std::ostream& stream);
        virtual message_type type() const = 0;
    
    protected:
        int m_http_major_;
        int m_http_minor_;
        
        message();
        message(transfer_encoding_token data);
        virtual std::string _M_start_line() = 0;
        
    private:
        std::vector<header_token> m_headers_;
        
        bool                    m_has_body_;
        bool                    m_is_fixed_body_;
        transfer_encoding_token m_data_;
        std::string             m_data_buffer_;
    };
}}

#endif
