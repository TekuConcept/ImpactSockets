/**
 * Created by TekuConcept on October 3, 2019
 */

#ifndef IMPACT_HTTP_MESSAGE_H
#define IMPACT_HTTP_MESSAGE_H

#include <vector>
#include <memory>
#include <iostream>

#include "rfc/http/header_list.h"
#include "rfc/http/message_traits.h"
#include "rfc/http/transfer_pipe.h"

namespace impact {
namespace http {

    class message_t {
    public:
        struct limits {
            size_t max_line_length;    // 8 kB
            size_t max_header_limit;   // 50 headers
            size_t chunk_size_limit;   // 1 MB
            limits();
        };

        static inline limits& message_limits() { return s_limits_; }
        static message_t get(std::string target="/");
        static message_t post(std::string target="/");

        /* [- generic ctors -] */
        message_t(
            std::unique_ptr<message_traits> traits,
            header_list headers = {},
            std::string body = "");
        // message_t(message_traits_ptr, transfer_encoding_token data);
        // message_t(message_traits_ptr, std::string data);
        /* [- request message convinience ctors -] */
        message_t(
            std::string method,
            std::string target,
            header_list headers = {},
            std::string body = "");
        message_t(
            method method,
            std::string target,
            header_list headers = {},
            std::string body = "");
        message_t(
            int status_code,
            std::string reason_phrase,
            header_list headers = {},
            std::string body = "");
        message_t(
            status_code status_code,
            header_list headers = {},
            std::string body = "");

        message_t(message_t&&);
        message_t(const message_t&) = delete;
        void operator=(const message_t&) = delete;

        virtual ~message_t() = default;

        std::string to_string() const;

        inline const message_traits* traits() const
        { return m_traits_.get(); }

        inline header_list& headers()
        { return m_headers_; }

        // ( not the same as payload body )
        // if a transfer pipe is specified, the body()
        // value here will be ignored because it will be
        // retrieved through the pipe
        inline std::string& body()
        { return m_body_; }

        inline std::shared_ptr<transfer_pipe>& pipe()
        { return m_pipe_; }

        message_t clone() const;

    private:
        std::unique_ptr<message_traits> m_traits_;
        header_list m_headers_;
        std::string m_body_;
        std::shared_ptr<transfer_pipe> m_pipe_;

        static limits s_limits_;

        message_t();

        friend std::ostream& operator<<(std::ostream&, const message_t&);
    };

}}

#endif
