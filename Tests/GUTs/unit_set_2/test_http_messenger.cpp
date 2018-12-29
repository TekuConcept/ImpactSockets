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

#define NO_THROW(code) try { {code} } catch (...) { FAIL(); }
#define THROW(code)    try { {code} FAIL(); } catch (...) { }

// TEST(test_http_messenger, request_valid_method)
// {
//     //  method = token
//     //  token  = 1*tchar
//     //  tchar  = "!" / "#" / "$" / "%" / "&" / "'" / "*"
//     //            / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"
//     //            / DIGIT / ALPHA
//     std::shared_ptr<http::request_message> request;
    
//     // [- GOOD -]
    
//     NO_THROW(request = http::request_message::create("GET", "/");)
    
//     // [- BAD -]
    
//     // no method
//     THROW(request = http::request_message::create("", "/");)
//     // invalid method character
//     THROW(request = http::request_message::create(" ", "/");)
// }


// TEST(test_http_messenger, request_valid_target)
// {
//     // request-target = origin-form
//     //                 / absolute-form
//     //                 / authority-form
//     //                 / asterisk-form
//     // origin-form    = absolute-path [ "?" query ]
//     // absolute-form  = absolute-URI
//     // authority-form = authority
//     // asterisk-form  = "*"
    
//     // Examples:
//     // origin-form:    /where?q=now
//     // absolute-form:  http://www.example.org/pub/WWW/TheProject.html
//     // authority-form: www.example.com:80
//     std::shared_ptr<http::request_message> request;
    
//     NO_THROW(request = http::request_message::create("A", "*");)
//     NO_THROW(request = http::request_message::create("A", "/");)
//     NO_THROW(request = http::request_message::create("A", "/where?q=now");)
//     NO_THROW(request = http::request_message::create("A",
//         "http://www.example.org/pub/WWW/TheProject.html");)
//     NO_THROW(request = http::request_message::create("A", "www.example.com:80");)
//     THROW(request = http::request_message::create("A", "//bad/origin");)
//     THROW(request = http::request_message::create("A", "");)
//     THROW(request = http::request_message::create("A", " ");)
// }


// TEST(test_http_messenger, transfer_encoding)
// {
//     // empty encoding set
//     // null data callback
//     // custom encoding
//     // duplicate chunked encodings
//     // duplicate other encodings
//     // custom encoding called "chunked" (hint: check name in ctor)
// }


// TEST(test_http_messenger, fixed_message)
// {
//     // create fixed-data message
// }


// TEST(test_http_messenger, headers)
// {
//     // generic header format
//     // add headers to message
//     // remove headers from message
//     // access message headers
//     // handle duplicate headers?
// }


// TEST(test_http_messenger, send_request)
// {
//     std::shared_ptr<http::request_message> request;
//     std::stringstream test_stream;
//     int finished = 0;
//     std::function<void(std::string*)> callback = 
//     [&](std::string* next) -> void {
//         if (!finished) *next = "Hello World!";
//         else           *next = "";
//         finished++;
//     };
    
//     // - basic request (no body) -
//     request = http::request_message::create("GET", "/");
//     request->send(test_stream);
//     EXPECT_EQ(test_stream.str(), "GET / HTTP/1.1\r\n\r\n");
//     test_stream.clear();
//     test_stream.str("");
    
//     // - advanced request (no body) -
//     request = http::request_message::create("GET", "/", {}, nullptr);
//     request->send(test_stream);
//     EXPECT_EQ(test_stream.str(), "GET / HTTP/1.1\r\n\r\n");
//     test_stream.clear();
//     test_stream.str("");
    
//     // - chunked body -
//     request = http::request_message::create("GET", "/", {}, callback);
//     request->send(test_stream);
//     EXPECT_EQ(test_stream.str(),
//         "GET / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n"
//         "c\r\nHello World!\r\n0\r\n\r\n");
//     test_stream.clear();
//     test_stream.str("");
//     finished = 0;
    
//     // - transfer codings -
//     http::transfer_encoding custom(
//         "custom",
//         [](const std::string& data) -> std::string { return data; }
//     );
//     request = http::request_message::create(
//         "GET", "/",
//         {
//             custom,
//             custom // duplicates allowed (except for chunked)
//         },
//         callback
//     );
//     request->send(test_stream);
//     EXPECT_EQ(test_stream.str(),
//         "GET / HTTP/1.1\r\nTransfer-Encoding: custom, custom, chunked\r\n\r\n"
//         "c\r\nHello World!\r\n0\r\n\r\n");
//     test_stream.clear();
//     test_stream.str("");
// }


// TEST(test_http_messenger, send_response)
// {
//     // auto response = http::response_message::create(200, "OK");
//     // response->send(std::cout);
// }


// TEST(test_http_messenger, parse_start_line)
// {
//     std::string input;
//     struct http::start_line start_line;
//     int status;

//     // - GOOD -
    
//     start_line.http_major = 0;
//     start_line.http_minor = 0;
//     input = "GET /hello.txt HTTP/1.1\r\n";
//     status = http::internal::parse_start_line(input, &start_line);
//     EXPECT_EQ(status, 0);
//     EXPECT_EQ(start_line.http_major, 1);
//     EXPECT_EQ(start_line.http_minor, 1);
//     EXPECT_EQ(start_line.type, http::message_type::REQUEST);
//     EXPECT_EQ(start_line.method, "GET");
//     EXPECT_EQ(start_line.target, "/hello.txt");
    
//     start_line.http_major = 0;
//     start_line.http_minor = 0;
//     input = "HTTP/1.1 200 OK\r\n";
//     status = http::internal::parse_start_line(input, &start_line);
//     EXPECT_EQ(status, 0);
//     EXPECT_EQ(start_line.http_major, 1);
//     EXPECT_EQ(start_line.http_minor, 1);
//     EXPECT_EQ(start_line.type, http::message_type::RESPONSE);
//     EXPECT_EQ(start_line.status, 200);
//     EXPECT_EQ(start_line.message, "OK");
    
//     // - BAD -
    
//     status = http::internal::parse_start_line(input, NULL);
//     EXPECT_EQ(status, -1);
    
//     std::vector<std::string> bad_start_lines = {
//         "GET  HTTP/1.1\r\n",   // no target
//         " / HTTP/1.1\r\n",     // no method
//         "GET / \r\n",          // no http version
//         "GET / HTTP/1.1",      // no CRLF at end of line (request)
//         "GET / HTP/1.1\r\n",   // mangled HTTP version
//         "GET \\ HTTP/1.1\r\n", // bad target
//         "GET /path /w/invalid/c hars HTTP/1.1\r\n", //bad target
//         "HTTP/1.1 abc OK\r\n", // bad status code
//         "HTTP/1.1 200\r\n",    // no SP between status code and CRLF
//         "HTTP/1.1 200 OK"      // no CRLF at end of line (response)
//     };
    
//     for (const auto& line : bad_start_lines) {
//         EXPECT_EQ(http::internal::parse_start_line(line, &start_line), -1);
//     }
// }


// TEST(test_http_messenger, parse_header_line)
// {
//     std::string input;
//     std::pair<std::string,std::string> header_line;
//     int status;
    
//     // - GOOD -
    
//     input = "Host: www.example.com\r\n";
//     status = http::internal::parse_header_line(input, &header_line);
//     EXPECT_EQ(status, 0);
//     EXPECT_EQ(header_line.first, "Host");
//     EXPECT_EQ(header_line.second, "www.example.com");
    
//     input = "Other:\r\n";
//     status = http::internal::parse_header_line(input, &header_line);
//     EXPECT_EQ(status, 0);
//     EXPECT_EQ(header_line.first, "Other");
//     EXPECT_EQ(header_line.second.size(), 0U);
    
//     // - BAD -
    
//     status = http::internal::parse_header_line(input, NULL);
//     EXPECT_EQ(status, -1);
    
//     std::vector<std::string> bad_header_lines = {
//         "Host : www.example.com\r\n", // space between field name and ':'
//         "Host: www.example.com", // no CRLF at end of line
//         ": www.example.com\r\n", // no field name
//     };
    
//     for (const auto& line : bad_header_lines) {
//         EXPECT_EQ(http::internal::parse_header_line(line, &header_line), -1);
//     }
// }


// TEST(test_http_messenger, fixed_body)
// {
//     http::fixed_body fixed_body_1;
//     http::fixed_body fixed_body_2("Hello World!");
//     fixed_body_1.data = "Hello World!";
//     http::body_format* body_format = &fixed_body_1;
//     EXPECT_EQ(body_format->type(), http::body_format_type::FIXED);
// }


// TEST(test_http_messenger, dynamic_body)
// {
//     std::stringstream foo("Hello World!");
//     http::dynamic_body dynamic_body_1;
//     http::dynamic_body dynamic_body_2(&foo, {});
//     dynamic_body_1.data_stream = &foo;
//     http::body_format* body_format = &dynamic_body_1;
//     EXPECT_EQ(body_format->type(), http::body_format_type::DYNAMIC);
// }


// TEST(test_http_messenger, create_request)
// {
//     http::request_message message("post", "www.example.com");
//     EXPECT_EQ(message.type(), http::message_type::REQUEST);
//     EXPECT_EQ(message.http_major(), 1);
//     EXPECT_EQ(message.http_minor(), 1);
//     EXPECT_EQ(message.method(), "POST");
//     EXPECT_EQ(message.target(), "www.example.com");
    
//     message.headers["Host"] = "localhost";
//     message.headers.insert({
//         {"User-Agent","ImpactSockets"},
//         {"Accept-Language","en"}
//     });
//     EXPECT_NE(message.headers.find("HOST"), message.headers.end());
//     EXPECT_NE(message.headers.find("ACCEPT-language"), message.headers.end());
    
//     EXPECT_EQ(message.body, nullptr);
//     message.body = std::shared_ptr<http::body_format>(
//         (http::body_format*)(new http::fixed_body("Hello World!"))
//     );
//     EXPECT_EQ(message.body->type(), http::body_format_type::FIXED);

//     std::stringstream ss("Hello World!");
//     message.body = std::shared_ptr<http::body_format>(
//         (http::body_format*)(new http::dynamic_body(&ss, {}))
//     );
//     EXPECT_EQ(message.body->type(), http::body_format_type::DYNAMIC);
// }


// // create_response


// TEST(test_http_messenger, send)
// {
//     http::request_message message("POST", "www.example.com");
//     message.headers["Host"] = "localhost";
//     message.headers.insert({
//         {"User-Agent","ImpactSockets"},
//         {"Accept-Language","en"}
//     });
//     message.send(&std::cout);
// }