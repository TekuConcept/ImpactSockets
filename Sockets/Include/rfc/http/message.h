/**
 * Created by TekuConcept on October 3, 2019
 */

#ifndef _IMPACT_HTTP_MESSAGE_H_
#define _IMPACT_HTTP_MESSAGE_H_

#include <vector>
#include <memory>
#include <iostream>

#include "rfc/http/header_list.h"
#include "rfc/http/message_traits.h"
#include "rfc/http/transfer_encoding.h"

namespace impact {
namespace http {

    class message_t {
    public:
        struct limits {
            unsigned int max_line_length    = 8000;    // 8 kB
            unsigned int max_header_limit   = 50;
            unsigned int payload_size_limit = 1000000; // 1 MB
            limits();
        };

        static message_t get(std::string target="/");
        static message_t post(std::string target="/");

        /* [- generic ctors -] */
        message_t(
            message_traits_ptr traits,
            header_list headers = {});
        // message_t(message_traits_ptr, transfer_encoding_token data);
        // message_t(message_traits_ptr, std::string data);
        /* [- request message convinience ctors -] */
        message_t(
            std::string method,
            std::string target,
            header_list headers = {});
        message_t(
            method method,
            std::string target,
            header_list headers = {});
        message_t(
            int status_code,
            std::string reason_phrase,
            header_list headers = {});
        message_t(
            status_code status_code,
            header_list headers = {});

        virtual ~message_t() = default;

        std::string to_string() const;

        inline const message_traits* traits() const
        { return m_traits_.get(); }

        inline header_list& headers()
        { return m_headers_; }

        // ( not the same as payload body )
        // if a transfer encoding is specified, the body()
        // value here will be ignored because it will be
        // retrieved through the encoding callbacks
        inline std::string& body()
        { return m_body_; }

        inline void set_transfer_encoding(
            std::shared_ptr<transfer_encoding> encoding)
        { m_encoding_ = encoding; }

        friend std::ostream& operator<<(std::ostream&, const message_t&);

    private:
        message_traits_ptr m_traits_;
        header_list m_headers_;
        std::string m_body_;
        std::shared_ptr<transfer_encoding> m_encoding_;

        message_t();
    };

}}

#endif
