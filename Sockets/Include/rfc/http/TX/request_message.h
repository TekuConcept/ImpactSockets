/**
 * Created by TekuConcept on December 28, 2018
 */

#ifndef _IMPACT_HTTP_REQUEST_MESSAGE_H_
#define _IMPACT_HTTP_REQUEST_MESSAGE_H_

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include "utils/case_string.h"
#include "rfc/http/TX/transfer_encoding.h"
#include "rfc/http/TX/message.h"

namespace impact {
namespace http {
    // caching object
    // (speed up execution by reducing validation checks)
    class method_token {
    public:
        method_token(std::string method_name);
        ~method_token();
    private:
        std::string m_name_;
        bool _M_valid_name(const std::string&) const;
    public:
        inline const std::string& name() const noexcept { return m_name_; }
    };
    
    
    // caching object
    // (speed up execution by reducing validation checks)
    class target_token {
    public:
        target_token(std::string target_name);
        ~target_token();
        enum class path_type { ASTERISK, ORIGIN, AUTHORITY, ABSOLUTE, UNKNOWN };
    private:
        std::string m_target_;
        path_type m_type_;
        path_type _M_valid_target(const std::string&) const;
    public:
        inline const std::string& name() const noexcept { return m_target_; }
        inline path_type type() const noexcept { return m_type_; }
    };
    
    
    class request_message : public message {
    public:
        request_message(method_token method, target_token target);
        request_message(method_token method, target_token target,
            transfer_encoding_token data);
        
        request_message(std::string method, std::string target)
            /* throw impact_error */;
        request_message(std::string method, std::string target,
            transfer_encoding_token::transfer_encoding_list encodings,
            std::function<void(std::string*)> data_callback)
            /* throw impact_error */;

        virtual ~request_message();

    private:
        method_token m_method_;
        target_token m_target_;
    
    public:
        inline const std::string& method() const noexcept
        { return m_method_.name(); }
        inline const std::string& target() const noexcept
        { return m_target_.name(); }
    };
}}

#endif
