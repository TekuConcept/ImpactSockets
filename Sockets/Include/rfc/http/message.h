/**
 * Created by TekuConcept on October 3, 2019
 */

#ifndef _IMPACT_HTTP_MESSAGE_H_
#define _IMPACT_HTTP_MESSAGE_H_

#include <vector>
#include <iostream>

#include "rfc/http/header_list.h"
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
        // if a transfer encoder is specified for this
        // message; the body() value here will be ignored
        // because it will be retrieved through the encoder
        inline std::string& body()
        { return m_body_; }

        /*
        A recipient MUST be able to parse the chunked transfer coding
        (Section 4.1) because it plays a crucial role in framing messages
        when the payload body size is not known in advance.  A sender MUST
        NOT apply chunked more than once to a message body (i.e., chunking an
        already chunked message is not allowed).  If any transfer coding
        other than chunked is applied to a request payload body, the sender
        MUST apply chunked as the final transfer coding to ensure that the
        message is properly framed.  If any transfer coding other than
        chunked is applied to a response payload body, the sender MUST either
        apply chunked as the final transfer coding or terminate the message
        by closing the connection.
        */
        // void set_transfer_encoders({});
        // - use last chunked encoder and discard all others
        // - set chunked encoder as last encoder

        friend std::ostream& operator<<(std::ostream&, const message_t&);

    private:
        message_traits_ptr m_traits_;
        header_list m_headers_;
        std::string m_body_;

        message_t();
    };

}}

#endif
