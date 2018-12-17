/**
 * Created by TekuConcept on December 15, 2018
 */

#include <string>
#include <sstream>
#include <vector>

#include <gtest/gtest.h>
#include "rfc/http/messenger.h"
#include "utils/environment.h"

using namespace impact;

TEST(test_http_messenger, parse_start_line)
{
    std::string input;
    struct http::start_line start_line;
    int status;

    // - GOOD -
    
    start_line.http_major = 0;
    start_line.http_minor = 0;
    input = "GET /hello.txt HTTP/1.1\r\n";
    status = http::internal::parse_start_line(input, &start_line);
    EXPECT_EQ(status, 0);
    EXPECT_EQ(start_line.http_major, 1);
    EXPECT_EQ(start_line.http_minor, 1);
    EXPECT_EQ(start_line.type, http::message_type::REQUEST);
    EXPECT_EQ(start_line.method, "GET");
    EXPECT_EQ(start_line.target, "/hello.txt");
    
    start_line.http_major = 0;
    start_line.http_minor = 0;
    input = "HTTP/1.1 200 OK\r\n";
    status = http::internal::parse_start_line(input, &start_line);
    EXPECT_EQ(status, 0);
    EXPECT_EQ(start_line.http_major, 1);
    EXPECT_EQ(start_line.http_minor, 1);
    EXPECT_EQ(start_line.type, http::message_type::RESPONSE);
    EXPECT_EQ(start_line.status, 200);
    EXPECT_EQ(start_line.message, "OK");
    
    // - BAD -
    
    status = http::internal::parse_start_line(input, NULL);
    EXPECT_EQ(status, -1);
    
    std::vector<std::string> bad_start_lines = {
        "GET  HTTP/1.1\r\n",   // no target
        " / HTTP/1.1\r\n",     // no method
        "GET / \r\n",          // no http version
        "GET / HTTP/1.1",      // no CRLF at end of line (request)
        "GET / HTP/1.1\r\n",   // mangled HTTP version
        "GET \\ HTTP/1.1\r\n", // bad target
        "GET /path /w/invalid/c hars HTTP/1.1\r\n", //bad target
        "HTTP/1.1 abc OK\r\n", // bad status code
        "HTTP/1.1 200\r\n",    // no SP between status code and CRLF
        "HTTP/1.1 200 OK"      // no CRLF at end of line (response)
    };
    
    for (const auto& line : bad_start_lines) {
        EXPECT_EQ(http::internal::parse_start_line(line, &start_line), -1);
    }
}


TEST(test_http_messenger, parse_header_line)
{
    std::string input;
    std::pair<std::string,std::string> header_line;
    int status;
    
    // - GOOD -
    
    input = "Host: www.example.com\r\n";
    status = http::internal::parse_header_line(input, &header_line);
    EXPECT_EQ(status, 0);
    EXPECT_EQ(header_line.first, "Host");
    EXPECT_EQ(header_line.second, "www.example.com");
    
    input = "Other:\r\n";
    status = http::internal::parse_header_line(input, &header_line);
    EXPECT_EQ(status, 0);
    EXPECT_EQ(header_line.first, "Other");
    EXPECT_EQ(header_line.second.size(), 0U);
    
    // - BAD -
    
    status = http::internal::parse_header_line(input, NULL);
    EXPECT_EQ(status, -1);
    
    std::vector<std::string> bad_header_lines = {
        "Host : www.example.com\r\n", // space between field name and ':'
        "Host: www.example.com", // no CRLF at end of line
        ": www.example.com\r\n", // no field name
    };
    
    for (const auto& line : bad_header_lines) {
        EXPECT_EQ(http::internal::parse_header_line(line, &header_line), -1);
    }
}


TEST(test_http_messenger, create_request)
{
    /*
    message = http::create_request(method, url);
    message.headers["field-name"] = "field-value";
    message.headers.insert({
        {"field1", "value1"},
        {"field2", "value2"}
    });
    message.body = "fixed-length message body";
    message.body = stream;
    
    message = http::create_response(code);
    */
    
    http::request_message message("POST", "www.example.com");
    EXPECT_EQ(message.type(), http::message_type::REQUEST);
    EXPECT_EQ(message.http_major(), 1);
    EXPECT_EQ(message.http_minor(), 1);
    EXPECT_EQ(message.method(), "POST");
    EXPECT_EQ(message.target(), "www.example.com");
}


TEST(test_http_messenger, send)
{
    http::request_message message("POST", "www.example.com");
    message.send(&std::cout);
}
