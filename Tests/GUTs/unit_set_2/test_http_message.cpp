/**
 * Created by TekuConcept on August 11, 2018
 */

#include <string>
#include <sstream>

#include <gtest/gtest.h>
#include <rfc/http/message.h>

using namespace impact;
using namespace http;

#define RESET(stream,string)\
    stream.clear();\
    stream.str(string);


std::string get_test_request() {
    std::ostringstream os;
    os << "GET /hello.txt HTTP/1.1\r\n";
    os << "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n";
    os << "Host: www.example.com\r\n";
    os << "Accept-Language: en, mi\r\n";
    os << "\r\n";
    return os.str();
}


std::string get_test_response() {
    std::ostringstream os;
    os << "HTTP/1.1 200 OK\r\n";
    os << "Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n";
    os << "Server: Apache\r\n";
    os << "Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n";
    os << "ETag: \"34aa387-d-1568eb00\"\r\n";
    os << "Accept-Ranges: bytes\r\n";
    os << "Content-Length: 51\r\n";
    os << "Vary: Accept-Encoding\r\n";
    os << "Content-Type: text/plain\r\n";
    os << "\r\n";
    os << "Hello World! My payload includes a trailing CRLF.\r\n";
    return os.str();
}


TEST(test_http_message, message) {
    std::stringstream stream;
    message_ptr msg;
    struct message_parser_opts opts;
    
    // null stream
    EXPECT_EQ(message::from_stream(NULL), nullptr);

    // non-null stream
    RESET(stream,get_test_response())
    msg = message::from_stream(&stream);
    EXPECT_EQ(msg->start_line(), "HTTP/1.1 200 OK");
    EXPECT_EQ(msg->header_fields().size(), 8);
    EXPECT_EQ(msg->message_body(),
        "Hello World! My payload includes a trailing CRLF.\r\n");

    // empty stream
    RESET(stream,"")
    EXPECT_EQ(message::from_stream(&stream), nullptr);
    
    RESET(stream,"\r\n")
    EXPECT_EQ(message::from_stream(&stream), nullptr);
    
    // no header tail
    RESET(stream,"HTTP/1.1 200 OK\r\n");
    EXPECT_EQ(message::from_stream(&stream), nullptr);
    
    // embeded linefeed
    RESET(stream,"HTTP/1.1\n200 OK\r\n\r\n");
    EXPECT_EQ(message::from_stream(&stream), nullptr);
    
    // embeded carriage return
    RESET(stream,"HTTP/1.1\r200 OK\r\n\r\n");
    EXPECT_EQ(message::from_stream(&stream), nullptr);
    
    // -: LIMITS :-
    
    // within limits
    RESET(stream,get_test_request());
    msg = message::from_stream(&stream, &opts);
    EXPECT_EQ(msg->start_line(), "GET /hello.txt HTTP/1.1");
    EXPECT_EQ(msg->header_fields().size(), 3);
    
    // line size limit
    opts.line_size_limit = 20; // default: 8000
    RESET(stream,"GET /long/path/to/a/resource HTTP/1.1\r\n\r\n");
    EXPECT_EQ(message::from_stream(&stream, &opts), nullptr);
    
    // header count limit
    opts.line_size_limit = 8000; // reset
    opts.header_count_limit = 4; // default: 101
    RESET(stream,get_test_response());
    EXPECT_EQ(message::from_stream(&stream, &opts), nullptr);
    
    // body size limit
    opts.header_count_limit = 4;
    opts.body_size_limit = 20; // default: 200000
    RESET(stream,get_test_response());
    EXPECT_EQ(message::from_stream(&stream, &opts), nullptr);
}


TEST(test_http_message, start_line) {
    std::stringstream stream;
    message_ptr msg;

    msg = message_ptr(new message());
    EXPECT_EQ(msg->type(), message_type::UNKNOWN);
    
    // -: VALIDITY :-
    
    // start line seen as empty
    RESET(stream,"\r\nHTTP/1.1 200 OK\r\n\r\n")
    EXPECT_EQ(message::from_stream(&stream), nullptr);
    
    // white space at begining of stream
    RESET(stream,"  HTTP/1.1 200 OK\r\n\r\n")
    EXPECT_EQ(message::from_stream(&stream), nullptr);
    
    // white space at end of stream
    RESET(stream,"HTTP/1.1 200 OK  \r\n\r\n")
    EXPECT_EQ(message::from_stream(&stream), nullptr);
    
    // more than three tokens
    RESET(stream,"HTTP/1.1 200 OK FOO\r\n\r\n")
    EXPECT_EQ(message::from_stream(&stream), nullptr);
    
    // successive whitespace characters
    RESET(stream,"HTTP/1.1  200  OK\r\n\r\n")
    EXPECT_EQ(message::from_stream(&stream), nullptr);

    // -: DETAILS :-

    // test message type detection
    // RESET(stream,"HTTP/1.1 200 OK\r\n\r\n");
    // msg = message::from_stream(&stream);
    // EXPECT_EQ(msg->type(), message_type::RESPONSE);
    
    // RESET(stream,"GET / HTTP/1.1\r\n\r\n");
    // msg = message::from_stream(&stream);
    // EXPECT_EQ(msg->type(), message_type::REQUEST);

    // RESET(stream,"HTTP / HTTP/1.1\r\n\r\n");
    // msg = message::from_stream(&stream);
    // EXPECT_EQ(msg->type(), message_type::REQUEST);
    
    // test http version major
    // test http version minor
    // test method
    // test request-target
    // test status-code
    // test status-message
}
