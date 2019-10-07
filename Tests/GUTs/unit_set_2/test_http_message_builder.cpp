/**
 * Created by TekuConcept on October 5, 2019
 */

#include <sstream>
#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "utils/environment.h"
#include "rfc/http/message_builder.h"

using namespace impact;
using namespace http;

#define NO_THROW_BEGIN try {
#define NO_THROW_END   } catch (...) { FAIL(); }
#define THROW_BEGIN   try {
#define THROW_END     FAIL(); } catch (...) { }


class message_builder_observer_proxy :
    public message_builder_observer {
public:
    virtual ~message_builder_observer_proxy() = default;
    void on_message(std::unique_ptr<message_t> message) override
    { on_message_proxy(message.get()); }
    virtual void on_message_proxy(message_t*) { };
};

class mock_message_builder_observer :
    public message_builder_observer_proxy {
public:
    MOCK_METHOD1(on_error, void(error_id));
    MOCK_METHOD1(on_message_proxy, void(message_t*));
    MOCK_METHOD1(on_data, void(const payload_fragment&));
};


TEST(test_http_message_builder, message_builder)
{
    using ::testing::_;
    mock_message_builder_observer observer;
    message_builder builder;

    builder.register_observer(&observer);

    // test fragmented message
    EXPECT_CALL(observer, on_message_proxy(_)).Times(0);
    EXPECT_CALL(observer, on_error(_)).Times(0);
    EXPECT_CALL(observer, on_data(_)).Times(0);
    builder.write("GET / ");
    EXPECT_CALL(observer, on_message_proxy(_)).Times(1);
    EXPECT_CALL(observer, on_error(_)).Times(0);
    EXPECT_CALL(observer, on_data(_)).Times(0);
    builder.write("HTTP/1.1\r\n\r\n");
    builder.clear();

    // test fragmented message with headers
    EXPECT_CALL(observer, on_message_proxy(_)).Times(0);
    EXPECT_CALL(observer, on_error(_)).Times(0);
    EXPECT_CALL(observer, on_data(_)).Times(0);
    builder.write("GET / HTTP/1.1\r\nHos");
    EXPECT_CALL(observer, on_message_proxy(_)).Times(1);
    EXPECT_CALL(observer, on_error(_)).Times(0);
    EXPECT_CALL(observer, on_data(_)).Times(0);
    builder.write("t: localhost\r\n\r\n");
    builder.clear();

    // test fragmented fixed body
    EXPECT_CALL(observer, on_message_proxy(_)).Times(0);
    EXPECT_CALL(observer, on_error(_)).Times(0);
    EXPECT_CALL(observer, on_data(_)).Times(0);
    builder.write("GET / HTTP/1.1\r\nContent-");
    EXPECT_CALL(observer, on_message_proxy(_)).Times(1);
    EXPECT_CALL(observer, on_error(_)).Times(0);
    EXPECT_CALL(observer, on_data(_)).Times(1);
    builder.write("length: 12\r\n\r\nH");
    EXPECT_CALL(observer, on_message_proxy(_)).Times(0);
    EXPECT_CALL(observer, on_error(_)).Times(0);
    EXPECT_CALL(observer, on_data(_)).Times(2);
    builder.write("ello Wo");
    builder.write("rld!");
    builder.clear();

    // test continuous request body
    EXPECT_CALL(observer, on_message_proxy(_)).Times(0);
    EXPECT_CALL(observer, on_error(_)).Times(1);
    EXPECT_CALL(observer, on_data(_)).Times(0);
    builder.write("GET / HTTP/1.1\r\nTransfer-Encoding: foo\r\n\r\n");
    builder.clear();

    // test continuous response body
    EXPECT_CALL(observer, on_message_proxy(_)).Times(1);
    EXPECT_CALL(observer, on_error(_)).Times(0);
    EXPECT_CALL(observer, on_data(_)).Times(0);
    builder.write("HTTP/1.1 200 OK\r\nTransfer-Encoding: foo\r\n\r\n");
    EXPECT_CALL(observer, on_message_proxy(_)).Times(0);
    EXPECT_CALL(observer, on_error(_)).Times(0);
    EXPECT_CALL(observer, on_data(_)).Times(2);
    builder.write("Hello World");
    builder.write("Good Bye World!");
    builder.clear();

    // TODO: test chunked body

    // test multiple body description headers
    EXPECT_CALL(observer, on_message_proxy(_)).Times(0);
    EXPECT_CALL(observer, on_error(_)).Times(1);
    EXPECT_CALL(observer, on_data(_)).Times(0);
    builder.write(
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: foo\r\n"
        "Transfer-Encoding: bar\r\n"
        "\r\n");
    builder.clear();

    // test pipelined messages
    EXPECT_CALL(observer, on_message_proxy(_)).Times(3);
    EXPECT_CALL(observer, on_error(_)).Times(0);
    EXPECT_CALL(observer, on_data(_)).Times(1);
    builder.write(
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 12\r\n"
        "\r\n"
        "Hello World!"
        "GET / HTTP/1.1\r\n\r\n"
        "TOUCH / HTTP/1.1\r\n\r\n");
    builder.clear();
}
