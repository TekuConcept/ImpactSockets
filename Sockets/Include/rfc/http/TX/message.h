/**
 * Created by TekuConcept on December 28, 2018
 */

#ifndef _IMPACT_HTTP_MESSAGE_H_
#define _IMPACT_HTTP_MESSAGE_H_

#include <string>
#include <vector>
#include <functional>
#include "utils/case_string.h"
#include "rfc/http/TX/message_header.h"
#include "rfc/http/TX/transfer_encoding.h"

namespace impact {
namespace http {
    // caching object
    // (speed up execution by reducing validation checks)
    class transfer_encoding_token {
    public:
        typedef std::shared_ptr<transfer_encoding> transfer_encoding_ptr;
        typedef std::vector<transfer_encoding_ptr> transfer_encoding_list;
        transfer_encoding_token(transfer_encoding_list encodings);
        transfer_encoding_token(transfer_encoding_list encodings,
            std::function<void(std::string*)> callback);
        ~transfer_encoding_token();
        std::function<void(std::string*)> callback;
    private:
        message_header m_header_;
        transfer_encoding_list m_transfer_encodings_;
    public:
        inline const transfer_encoding_list& encodings()
        { return m_transfer_encodings_; }
        inline const message_header& header() { return m_header_; }
    };

    
    enum class message_type { REQUEST, RESPONSE };
    
    
    class message {
    public:
        message() = delete;
        virtual ~message();
        message_type type() const noexcept;
        
        void send(std::ostream& stream);
    
    protected:
        message(message_type type);
        message(message_type type, transfer_encoding_token data);
        
    private:
        message_type m_type_;
        int m_http_major_;
        int m_http_minor_;
        
        std::vector<message_header> m_headers_;
        
        bool                    m_has_body_;
        bool                    m_is_fixed_body_;
        transfer_encoding_token m_data_;
        std::string             m_data_buffer_;
    };
}}

#endif
