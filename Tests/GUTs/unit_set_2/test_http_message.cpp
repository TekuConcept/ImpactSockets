/**
 * Created by TekuConcept on December 29, 2018
 */

#include <gtest/gtest.h>
#include "utils/environment.h"
#include "rfc/http/TX/message.h"

using namespace impact;
using namespace http;

#define NO_THROW(code) try { {code} } catch (...) { FAIL(); }
#define THROW(code)    try { {code} FAIL(); } catch (...) { }

TEST(test_http_transfer_encoding_token, transfer_encoding_token)
{
    {
        transfer_encoding_token token({}, nullptr);
        ASSERT_EQ(token.encodings().size(), 1UL);
        EXPECT_EQ(token.encodings()[0]->name(), "chunked");
        auto header = token.header();
        EXPECT_EQ(header.field_name(), "Transfer-Encoding");
        EXPECT_EQ(header.field_value(), "chunked");
        EXPECT_EQ(token.callback, nullptr);
    }
    
    {
        transfer_encoding_token token({
            transfer_encoding::chunked(),
            transfer_encoding::chunked()
        }, nullptr);
        ASSERT_EQ(token.encodings().size(), 1UL);
        EXPECT_EQ(token.encodings()[0]->name(), "chunked");
    }

    transfer_encoding_ptr custom = std::make_shared<transfer_encoding>(
        "custom",
        [](const std::string& data) -> std::string { return data; }
    );
    
    {
        transfer_encoding_token token({ custom, custom }, nullptr);
        ASSERT_EQ(token.encodings().size(), 3UL);
        EXPECT_EQ(token.encodings()[0]->name(), "custom");
        EXPECT_EQ(token.encodings()[1]->name(), "custom");
        EXPECT_EQ(token.encodings()[2]->name(), "chunked");
        auto header = token.header();
        EXPECT_EQ(header.field_name(), "Transfer-Encoding");
        EXPECT_EQ(header.field_value(), "custom, custom, chunked");
    }
}


TEST(test_http_message, create)
{
    // [- create all ctor parameters used -]
    std::function<void(std::string*)> callback = 
        [&](std::string* next) -> void { *next = ""; };
    std::vector<transfer_encoding_ptr> encodings =
        { transfer_encoding::chunked() };
    transfer_encoding_token dynamic(encodings, callback);
    std::string fixed = "Hello World!";
    int status_code = 200;
    std::string reason_phrase = "OK";
    std::string str_method = "GET";
    std::string str_target = "/";
    method_token method(str_method);
    target_token target(str_target);
    message_traits_ptr traits((message_traits*)
        (new request_traits("GET","/")));
    
    // [- generic message constructors -]
    NO_THROW(message foo = message(traits);)
    NO_THROW(message foo = message(traits, dynamic);)
    NO_THROW(message foo = message(traits, fixed);)
    
    // [- request convinience constructors -]
    NO_THROW(message foo = message(str_method, str_target);)
    NO_THROW(message foo = message(str_method, str_target, encodings, callback);)
    NO_THROW(message foo = message(str_method, str_target, fixed);)
    NO_THROW(message foo = message(method, target, dynamic);)
    NO_THROW(message foo = message(method, target, fixed);)
    
    // [- response convinience constructors -]
    NO_THROW(message foo = message(status_code, reason_phrase);)
    NO_THROW(message foo = message(status_code, reason_phrase, encodings, callback);)
    NO_THROW(message foo = message(status_code, reason_phrase, dynamic);)
    NO_THROW(message foo = message(status_code, reason_phrase, fixed);)
}


TEST(test_http_message, send)
{
    std::stringstream test_stream;
    int finished = 0;
    std::function<void(std::string*)> callback = 
    [&](std::string* next) -> void {
        if (!finished) *next = "Hello World!";
        else           *next = "";
        finished++;
    };
    
    message_traits_ptr req_traits = message_traits_ptr(
        (message_traits*)(new request_traits("GET", "/"))
    );
    message_traits_ptr res_traits = message_traits_ptr(
        (message_traits*)(new response_traits(200, "OK"))
    );
    
    // - basic request (no body) -
    message request(req_traits);
    request.send(test_stream);
    EXPECT_EQ(test_stream.str(), "GET / HTTP/1.1\r\n\r\n");
    test_stream.clear();
    test_stream.str("");
    
    // - basic response (no body) -
    message response(res_traits);
    response.send(test_stream);
    EXPECT_EQ(test_stream.str(), "HTTP/1.1 200 OK\r\n\r\n");
    test_stream.clear();
    test_stream.str("");
    
    // - advanced request (no body) -
    request = message(req_traits, transfer_encoding_token({},nullptr));
    request.send(test_stream);
    EXPECT_EQ(test_stream.str(), "GET / HTTP/1.1\r\n\r\n");
    test_stream.clear();
    test_stream.str("");
    
    // - chunked body -
    request = message(req_traits, transfer_encoding_token({},callback));
    request.send(test_stream);
    EXPECT_EQ(test_stream.str(),
        "GET / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n"
        "c\r\nHello World!\r\n0\r\n\r\n");
    test_stream.clear();
    test_stream.str("");
    finished = 0;
    
    // - fixed body -
    request = message(req_traits, "Hello World!");
    request.send(test_stream);
    EXPECT_EQ(test_stream.str(),
        "GET / HTTP/1.1\r\nContent-Length: 12\r\n\r\nHello World!");
    test_stream.clear();
    test_stream.str("");
}


TEST(test_http_message, headers)
{
    NO_THROW( // add generic headers
        std::stringstream stream;
        message foo = message("GET", "/");
        foo.headers({
            header_token("Host", "www.example.com")
        });
        EXPECT_EQ(foo.headers().size(), 1UL);
        foo.send(stream);
        EXPECT_EQ(stream.str(),
            "GET / HTTP/1.1\r\n"
            "Host: www.example.com\r\n"
            "\r\n");
    )
    
    NO_THROW( // length header is prepended to header list
        std::stringstream stream;
        message foo = message("GET", "/", "Hello World!");
        foo.headers({
            header_token("Host", "www.example.com"),
            /* will be overwritten or ignored */
            header_token("Content-Length", "5")
        });
        EXPECT_EQ(foo.headers().size(), 2UL);
        foo.send(stream);
        EXPECT_EQ(stream.str(),
            "GET / HTTP/1.1\r\n"
            "Content-Length: 12\r\n"
            "Host: www.example.com\r\n"
            "\r\n"
            "Hello World!");
    )
    
    THROW(
        // duplicate length headers not allowed
        // NOTE: only throws if user length headers are permitted,
        //       otherwise all length headers are ignore including
        //       duplicates
        message foo = message(304, "Not Modified");
        foo.headers({
            header_token("Content-Length", "0"),
            header_token("Content-Length", "5")
        });
    )
    
}
