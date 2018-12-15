/**
 * Created by TekuConcept on December 15, 2018
 */

#include <string>
#include <vector>

#include <gtest/gtest.h>
#include "rfc/http/parser.h"
#include "rfc/http/types.h"

using namespace impact;

TEST(test_http_parser, parse_start_line) {
    std::string input;
    struct http::start_line start_line;
    int status;

    // - GOOD -
    
    start_line.http_major = 0;
    start_line.http_minor = 0;
    input = "GET /hello.txt HTTP/1.1\r\n";
    status = http::parse_start_line(input, &start_line);
    EXPECT_EQ(status, 0);
    EXPECT_EQ(start_line.http_major, 1);
    EXPECT_EQ(start_line.http_minor, 1);
    EXPECT_EQ(start_line.type, http::message_type::REQUEST);
    EXPECT_EQ(start_line.method, "GET");
    EXPECT_EQ(start_line.target, "/hello.txt");
    
    start_line.http_major = 0;
    start_line.http_minor = 0;
    input = "HTTP/1.1 200 OK\r\n";
    status = http::parse_start_line(input, &start_line);
    EXPECT_EQ(status, 0);
    EXPECT_EQ(start_line.http_major, 1);
    EXPECT_EQ(start_line.http_minor, 1);
    EXPECT_EQ(start_line.type, http::message_type::RESPONSE);
    EXPECT_EQ(start_line.status, 200);
    EXPECT_EQ(start_line.message, "OK");
    
    // - BAD -
    
    status = http::parse_start_line(input, NULL);
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
        EXPECT_EQ(http::parse_start_line(line, &start_line), -1);
    }
}
