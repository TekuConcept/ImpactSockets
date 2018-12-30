/**
 * Created by TekuConcept on December 28, 2018
 */

#ifndef _IMPACT_HTTP_RESPONSE_MESSAGE_H_
#define _IMPACT_HTTP_RESPONSE_MESSAGE_H_

#include <string>
#include <memory>
#include "rfc/http/TX/message.h"

namespace impact {
namespace http {
    class response_message : public message {
    public:
        response_message(int status_code, std::string reason_phrase);
        response_message(status_code);
        virtual ~response_message();
        message_type type() const;
        
    private:
        int m_status_code_;
        std::string m_reason_phrase_;
        std::string _M_start_line();
        
        bool _M_valid_phrase(const std::string&) const;
    
    public:
        inline int status_code() const noexcept
        { return m_status_code_; }
        inline const std::string& reason_phrase() const noexcept
        { return m_reason_phrase_; }
    };
}}

#endif
