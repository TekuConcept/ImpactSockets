/**
 * Created by TekuConcept on December 30, 2018
 */

#ifndef _IMPACT_HTTP_MESSAGE_TRAITS_H_
#define _IMPACT_HTTP_MESSAGE_TRAITS_H_

#include <string>
#include <memory>
#include "rfc/http/types.h"

namespace impact {
namespace http {
    enum class message_type { REQUEST, RESPONSE, NONE };
    
    
    class message_traits {
    public:
        static std::shared_ptr<message_traits> create(std::string line);
        virtual ~message_traits();
        virtual message_type type() const noexcept = 0;
        virtual std::string start_line() const noexcept = 0;
        virtual bool permit_user_length_header() const noexcept = 0;
        virtual bool permit_length_header() const noexcept = 0;
        virtual bool permit_body() const noexcept = 0;
    protected:
        int m_http_major_;
        int m_http_minor_;
    public:
        inline int http_major() const noexcept { return m_http_major_; }
        inline int http_minor() const noexcept { return m_http_minor_; }
    };
    typedef std::shared_ptr<message_traits> message_traits_ptr;
    
    
    class method_token {
    public:
        method_token(std::string method_name);
        method_token(method id);
        ~method_token();
    private:
        std::string m_name_;
        method_token();
        bool _M_valid_name(const std::string&) const;
    public:
        inline const std::string& name() const noexcept { return m_name_; }
        friend class request_traits;
        friend class message_traits;
    };
    
    
    class target_token {
    public:
        target_token(std::string target_name);
        ~target_token();
        enum class path_type { ASTERISK, ORIGIN, AUTHORITY, ABSOLUTE, UNKNOWN };
    private:
        std::string m_target_;
        path_type m_type_;
        target_token();
        path_type _M_valid_target(const std::string&) const;
    public:
        inline const std::string& name() const noexcept { return m_target_; }
        inline path_type type() const noexcept { return m_type_; }
        friend class request_traits;
    };
    
    
    class request_traits : public message_traits {
    public:
        request_traits(std::string method, std::string target);
        request_traits(method_token method, target_token target);
        ~request_traits();        
    
        message_type type() const noexcept;
        std::string start_line() const noexcept;
        bool permit_user_length_header() const noexcept;
        bool permit_length_header() const noexcept;
        bool permit_body() const noexcept;
        
    private:
        method_token m_method_;
        target_token m_target_;
        
        request_traits();
    
    public:
        inline const std::string& method() const noexcept
        { return m_method_.name(); }
        inline const std::string& target() const noexcept
        { return m_target_.name(); }
    
    friend class message_traits;
    };
    
    
    class response_traits : public message_traits {
    public:
        response_traits(int status_code, std::string reason_phrase);
        response_traits(status_code id);
        ~response_traits();
    
        message_type type() const noexcept;
        std::string start_line() const noexcept;
        bool permit_user_length_header() const noexcept;
        bool permit_length_header() const noexcept;
        bool permit_body() const noexcept;
    
    private:
        int         m_status_code_;
        std::string m_reason_phrase_;
        
        response_traits();
        void _M_validate();
        bool _M_valid_phrase(const std::string& __data) const noexcept;
    
    public:
        inline int code() const noexcept
        { return m_status_code_; }
        inline const std::string& reason_phrase() const noexcept
        { return m_reason_phrase_; }
    
    friend class message_traits;
    };
}}

#endif
