/**
 * Created by TekuConcept on December 30, 2018
 */

#ifndef _IMPACT_HTTP_MESSAGE_TRAITS_H_
#define _IMPACT_HTTP_MESSAGE_TRAITS_H_

#include <iostream>
#include <string>
#include <memory>
#include "rfc/http/types.h"

namespace impact {
namespace http {

    class message_traits {
    public:
        enum class message_type { UNKNOWN, REQUEST, RESPONSE };

        static std::shared_ptr<message_traits> create(std::string line);
        virtual ~message_traits();

        virtual message_type type() const noexcept = 0;
        virtual std::string to_string() const noexcept = 0;
        virtual bool permit_user_length_header() const noexcept = 0;
        virtual bool permit_length_header() const noexcept = 0;
        virtual bool permit_body() const noexcept = 0;

        inline int http_major() const noexcept { return m_http_major_; }
        inline int http_minor() const noexcept { return m_http_minor_; }


        class method_t {
        public:
            method_t(std::string method_name);
            method_t(method id);
            ~method_t();

            inline const std::string& name() const noexcept { return m_name_; }

        private:
            std::string m_name_;
            method_t();
            bool _M_valid_name(const std::string&) const;

            friend class request_traits;
            friend class message_traits;
        };


        class target_t {
        public:
            enum class path_type { ASTERISK, ORIGIN, AUTHORITY, ABSOLUTE, UNKNOWN };

            target_t(std::string target_name);
            ~target_t();

            inline const std::string& name() const noexcept { return m_target_; }
            inline path_type type() const noexcept { return m_type_; }

        private:
            std::string m_target_;
            path_type m_type_;

            target_t();
            path_type _M_valid_target(const std::string&) const;

            friend class request_traits;
        };

    protected:
        int m_http_major_;
        int m_http_minor_;
    };
    typedef std::shared_ptr<message_traits> message_traits_ptr;


    class request_traits : public message_traits {
    public:
        request_traits(std::string method, std::string target);
        request_traits(method_t method, target_t target);
        ~request_traits();

        message_type type() const noexcept;
        std::string to_string() const noexcept;
        bool permit_user_length_header() const noexcept;
        bool permit_length_header() const noexcept;
        bool permit_body() const noexcept;

        inline const std::string& method() const noexcept
        { return m_method_.name(); }
        inline const std::string& target() const noexcept
        { return m_target_.name(); }

        friend std::ostream& operator<<(std::ostream&, const request_traits&);

    private:
        method_t m_method_;
        target_t m_target_;

        request_traits();

        friend class message_traits;
    };


    class response_traits : public message_traits {
    public:
        response_traits(int status_code, std::string reason_phrase);
        response_traits(status_code id);
        ~response_traits();

        message_type type() const noexcept;
        std::string to_string() const noexcept;
        bool permit_user_length_header() const noexcept;
        /* RFC 7230 - 3.3.1
        A server MUST NOT send a Transfer-Encoding header field in any
        response with a status code of 1xx (Informational) or 204 (No
        Content).  A server MUST NOT send a Transfer-Encoding header field in
        any 2xx (Successful) response to a CONNECT request
        */
        bool permit_length_header() const noexcept;
        bool permit_body() const noexcept;

        inline int code() const noexcept
        { return m_status_code_; }
        inline const std::string& reason_phrase() const noexcept
        { return m_reason_phrase_; }

        friend std::ostream& operator<<(std::ostream&, const response_traits&);

    private:
        int         m_status_code_;
        std::string m_reason_phrase_;

        response_traits();
        void _M_validate();
        bool _M_valid_phrase(const std::string& __data) const noexcept;

        friend class message_traits;
    };
}}

#endif
