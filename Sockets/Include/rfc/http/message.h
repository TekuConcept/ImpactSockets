/**
 * Created by TekuConcept on October 3, 2019
 */

#ifndef _IMPACT_HTTP_MESSAGE_H_
#define _IMPACT_HTTP_MESSAGE_H_

#include <vector>
#include <iostream>

#include "rfc/http/header.h"
#include "rfc/http/message_traits.h"

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
        // message_t(std::string method, std::string target,
        //     transfer_encoding_token::list encodings,
        //     std::function<void(std::string*)>&& data_callback);
        // message_t(
        //     message_traits::method_t method,
        //     message_traits::target_t target,
        //     transfer_encoding_token data);
        // message_t(
        //     std::string method,
        //     std::string target,
        //     std::string data);
        // message_t(
        //     message_traits::method_t method,
        //     message_traits::target_t target,
        //     std::string data);
        /* [- response message convinience ctors -] */
        message_t(
            int status_code,
            std::string reason_phrase,
            header_list headers = {});
        message_t(
            status_code status_code,
            header_list headers = {});
        // message_t(int status_code, std::string reason_phrase,
        //     transfer_encoding_token::list encodings,
        //     std::function<void(std::string*)>&& data_callback);
        // message_t(
        //     int status_code,
        //     std::string reason_phrase,
        //     transfer_encoding_token data);
        // message_t(int status_code, std::string reason_phrase, std::string data);

        virtual ~message_t() = default;

        std::string to_string() const;
        inline const message_traits* traits() const
        { return m_traits_.get(); }

        /*  RFC7230 - 3.2.2
            A sender MUST NOT generate multiple header fields with the same field
            name in a message unless either the entire field value for that
            header field is defined as a comma-separated list [i.e., #(values)]
            or the header field is a well-known exception
        */
        inline header_list& headers()
        { return m_headers_; }

        friend std::ostream& operator<<(std::ostream&, const message_t&);

    private:
        message_traits_ptr m_traits_;
        header_list m_headers_;

        message_t();
    };

}}

#endif
