/**
 * Created by TekuConcept on October 3, 2019
 */

#include <gtest/gtest.h>
#include "utils/environment.h"
#include "rfc/http/message.h"

using namespace impact;
using namespace http;

#define NO_THROW_BEGIN try {
#define NO_THROW_END   } catch (...) { FAIL(); }
#define THROW_BEGIN   try {
#define THROW_END     FAIL(); } catch (...) { }

TEST(test_http_message, message_t)
{
    NO_THROW_BEGIN
        message_t message("GET", "/test");
        EXPECT_EQ(message.to_string(), "GET /test HTTP/1.1\r\n\r\n");
    NO_THROW_END

    NO_THROW_BEGIN
        message_t message(method::GET, "/test");
        EXPECT_EQ(message.to_string(), "GET /test HTTP/1.1\r\n\r\n");
    NO_THROW_END

    NO_THROW_BEGIN
        message_t message(
            "GET"
            , "/test"
            , {
                header_t(field_name::HOST, "localhost"),
                header_t(field_name::ACCEPT_ENCODING, "utf8")
            }
        );
        EXPECT_EQ(message.to_string(),
            "GET /test HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Accept-Encoding: utf8\r\n"
            "\r\n");
    NO_THROW_END

    NO_THROW_BEGIN
        message_t message(200, "OK");
        EXPECT_EQ(message.to_string(), "HTTP/1.1 200 OK\r\n\r\n");
    NO_THROW_END

    NO_THROW_BEGIN
        message_t message(status_code::OK);
        EXPECT_EQ(message.to_string(), "HTTP/1.1 200 OK\r\n\r\n");
    NO_THROW_END
}


TEST(test_http_message, simple_body)
{
    // Content-Length and Transfer-Encoding headers are
    // automatically generated based on how the message
    // is configured.
    //
    // Content-Length is generated when no transfer
    // encoder is specified and the body has non-zero
    // length.
    //
    // Transfer-Encoding is generated when a transfer
    // encoder is specified regardless of the body's size.

    // non-empty body but no Content-Length header
    NO_THROW_BEGIN
        message_t message = message_t::get();
        message.body() = "Hello World!";
        EXPECT_EQ(message.to_string(),
            "GET / HTTP/1.1\r\n"
            "Content-Length: 12\r\n"
            "\r\n"
            "Hello World!");
    NO_THROW_END

    // Content-Length header but no body
    // (includes Content-Length of 0)
    NO_THROW_BEGIN
        message_t message = message_t::get();
        message.headers().push_back(
            header_t(field_name::CONTENT_LENGTH, std::to_string(0)));
        EXPECT_EQ(message.to_string(), "GET / HTTP/1.1\r\n\r\n");
    NO_THROW_END

    // Content-Length header size differs from body size
    NO_THROW_BEGIN
        message_t message = message_t::get();
        message.headers().push_back(
            header_t(field_name::CONTENT_LENGTH, std::to_string(0)));
        message.body() = "Hello World!";
        EXPECT_EQ(message.to_string(),
            "GET / HTTP/1.1\r\n"
            "Content-Length: 12\r\n"
            "\r\n"
            "Hello World!");
    NO_THROW_END

    // message type that does not permit message body
    NO_THROW_BEGIN
        message_t message(method::HEAD, "/");
        message.body() = "Hello World!";
        EXPECT_EQ(message.to_string(),
            "HEAD / HTTP/1.1\r\n"
            "Content-Length: 12\r\n"
            "\r\n");
    NO_THROW_END

    // message type that does not permit message body
    // or message body headers ie Content-Length or
    // Transfer-Encoding
    NO_THROW_BEGIN
        message_t message(status_code::NO_CONTENT);
        message.body() = "Hello World!";
        EXPECT_EQ(message.to_string(),
            "HTTP/1.1 204 No Content\r\n"
            "\r\n");
    NO_THROW_END
}


TEST(test_http_message, clone)
{
    NO_THROW_BEGIN
        message_t message(
            "GET", "/test",
            {
                header_t(field_name::HOST, "localhost"),
                header_t(field_name::ACCEPT_ENCODING, "utf8")
            }
        );
        message_t clone = message.clone();
        EXPECT_EQ(clone.to_string(),
            "GET /test HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Accept-Encoding: utf8\r\n"
            "\r\n");
    NO_THROW_END
}


class dummy_connection {
public:
    dummy_connection(std::iostream& stream)
    : m_stream_(stream) {}

    void
    send(std::unique_ptr<message_t> message)
    {
        if (message == nullptr) return;
        m_stream_ << message->to_string();
        if (message->pipe()) {
            m_message_ = std::move(message);
            m_message_->pipe()->set_sink([&](const std::string& __chunk) {
                m_stream_ << __chunk;
                // TODO: more elegantly reset m_message_ on end of message
                if (__chunk.size() == 0 || __chunk[0] == '0')
                    m_message_.reset(nullptr);
            });
        }
    }

private:
    std::iostream& m_stream_;
    std::unique_ptr<message_t> m_message_;
};


TEST(test_http_message, piped_body)
{
    std::stringstream stream;
    dummy_connection connection(stream);

    message_t* message = new message_t("GET", "/", {
        header_t(field_name::HOST, "www.example.com")
    });

    auto* pipe = new transfer_pipe();
    message->pipe() = std::unique_ptr<transfer_pipe>(pipe);

    connection.send(std::unique_ptr<message_t>(message));

    pipe->send("Hello World!");
    pipe->send(pipe->eop());

    // TODO: safe-guard against segfault
    //       pipe is owned by message, which is automatically
    //       deleted after the EOP signal is sent
    // TODO: lock pipe sync - the pipe's set_sync must not be
    //       altered by any object other than the current owner
    //       while an outbound payload is in progress

    EXPECT_EQ(stream.str(),
        "GET / HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "C\r\nHello World!\r\n"
        "0\r\n"
        "\r\n");
}
