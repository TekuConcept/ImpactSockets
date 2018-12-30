/**
 * Created by TekuConcept on December 15, 2018
 */

#include <string>
#include <sstream>
#include <vector>

#include <gtest/gtest.h>
#include "utils/environment.h"
#include "rfc/http/TX/request_message.h"

using namespace impact;
using namespace http;

#define NO_THROW(code) try { {code} } catch (...) { FAIL(); }
#define THROW(code)    try { {code} FAIL(); } catch (...) { }

TEST(test_http_method_token, method_token)
{
    //  method = token
    //  token  = 1*tchar
    //  tchar  = "!" / "#" / "$" / "%" / "&" / "'" / "*"
    //            / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"
    //            / DIGIT / ALPHA
    
    // [- GOOD -]
    
    NO_THROW(method_token token = method_token("GET");)
    
    // [- BAD -]
    
    // no method
    THROW(method_token token = method_token("");)
    // invalid method character
    THROW(method_token token = method_token(" ");)
    
    // [- INFO -]
    NO_THROW(
        method_token token = method_token("GET");
        EXPECT_EQ(token.name(), "GET");
    )
    NO_THROW( // skip method name validation check
        method_token token = method_token(method::CONNECT);
        EXPECT_EQ(token.name(), "CONNECT");
    )
}


TEST(test_http_target_token, target_token)
{
    // request-target = origin-form
    //                 / absolute-form
    //                 / authority-form
    //                 / asterisk-form
    // origin-form    = absolute-path [ "?" query ]
    // absolute-form  = absolute-URI
    // authority-form = authority
    // asterisk-form  = "*"
    
    // Examples:
    // origin-form:    /where?q=now
    // absolute-form:  http://www.example.org/pub/WWW/TheProject.html
    // authority-form: www.example.com:80
    
    NO_THROW(
        target_token token = target_token("*");
        EXPECT_EQ(token.type(), target_token::path_type::ASTERISK);
        EXPECT_EQ(token.name(), "*");
    )
    NO_THROW(
        target_token token = target_token("/");
        EXPECT_EQ(token.type(), target_token::path_type::ORIGIN);
        EXPECT_EQ(token.name(), "/");
    )
    NO_THROW(
        target_token token = target_token("/where?q=now");
        EXPECT_EQ(token.type(), target_token::path_type::ORIGIN);
        EXPECT_EQ(token.name(), "/where?q=now");
    )
    NO_THROW(
        target_token token = target_token(
            "http://www.example.org/pub/WWW/TheProject.html");
        EXPECT_EQ(token.type(), target_token::path_type::ABSOLUTE);
        EXPECT_EQ(token.name(),
            "http://www.example.org/pub/WWW/TheProject.html");
    )
    NO_THROW(
        target_token token = target_token("www.example.com:80");
        EXPECT_EQ(token.type(), target_token::path_type::AUTHORITY);
        EXPECT_EQ(token.name(), "www.example.com:80");
    )
    THROW(target_token token = target_token("//bad/origin");)
    THROW(target_token token = target_token("");)
    THROW(target_token token = target_token(" ");)
}


TEST(test_http_request_message, valid_method)
{
    // NOTE: See also method_token
    NO_THROW(request_message request = request_message("GET", "/");)
    THROW(request_message request = request_message("", "/");)
}


TEST(test_http_request_message, valid_target)
{
    // NOTE: See also target_token
    NO_THROW(request_message request = request_message("_", "/");)
    THROW(request_message request = request_message("_", "//bad/origin");)
}


TEST(test_http_request_message, start_line)
{
    NO_THROW(
        request_message request = request_message("GET", "/");
        std::stringstream stream;
        request.send(stream);
        EXPECT_EQ(stream.str(), "GET / HTTP/1.1\r\n\r\n");
    )
}


TEST(test_http_request_message, type)
{
    NO_THROW(
        request_message request = request_message("GET", "/");
        EXPECT_EQ(request.type(), message_type::REQUEST);
    )
}


TEST(test_http_request_message, send_request)
{
    std::stringstream test_stream;
    int finished = 0;
    std::function<void(std::string*)> callback = 
    [&](std::string* next) -> void {
        if (!finished) *next = "Hello World!";
        else           *next = "";
        finished++;
    };
    
    // - basic request (no body) -
    request_message request("GET", "/");
    request.send(test_stream);
    EXPECT_EQ(test_stream.str(), "GET / HTTP/1.1\r\n\r\n");
    test_stream.clear();
    test_stream.str("");
    
    // - advanced request (no body) -
    request = request_message("GET", "/", {}, nullptr);
    request.send(test_stream);
    EXPECT_EQ(test_stream.str(), "GET / HTTP/1.1\r\n\r\n");
    test_stream.clear();
    test_stream.str("");
    
    // - chunked body -
    request = request_message("GET", "/", {}, callback);
    request.send(test_stream);
    EXPECT_EQ(test_stream.str(),
        "GET / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n"
        "c\r\nHello World!\r\n0\r\n\r\n");
    test_stream.clear();
    test_stream.str("");
    finished = 0;
    
    // // - transfer codings -
    // std::shared_ptr<http::transfer_encoding> custom =
    // std::make_shared<http::transfer_encoding>(
    //     "custom",
    //     [](const std::string& data) -> std::string { return data; }
    // );
    // request = request_message(
    //     "GET", "/",
    //     {
    //         custom,
    //         custom // duplicates allowed (except for chunked)
    //     },
    //     callback
    // );
    // request.send(test_stream);
    // EXPECT_EQ(test_stream.str(),
    //     "GET / HTTP/1.1\r\nTransfer-Encoding: custom, custom, chunked\r\n\r\n"
    //     "c\r\nHello World!\r\n0\r\n\r\n");
    // test_stream.clear();
    // test_stream.str("");
}
