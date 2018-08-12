/**
 * Created by TekuConcept on August 11, 2018
 */

#include <string>
#include <sstream>

#include <gtest/gtest.h>
#include <rfc/http/message.h>

using namespace impact;
using namespace http;


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
    /*
    HTTP-message = start-line
                   *( header-field CRLF )
                   CRLF
                   [ message-body ]
    */
    std::stringstream stream(get_test_response());
    message msg;
    struct message_parser_opts opts;
    
    // null stream
    msg = message::from_stream(NULL);
    EXPECT_FALSE(msg.valid());
    
    // non-null stream
    msg = message::from_stream(&stream);
    EXPECT_EQ(msg.start_line(), "HTTP/1.1 200 OK");
    EXPECT_EQ(msg.header_fields().size(), 8);
    EXPECT_EQ(msg.message_body(),
        "Hello World! My payload includes a trailing CRLF.\r\n");
    EXPECT_TRUE(msg.valid());
    
    // empty stream
    stream.clear();
    stream.str("");
    msg = message::from_stream(&stream);
    EXPECT_FALSE(msg.valid());
    
    stream.clear();
    stream.str("\r\n");
    msg = message::from_stream(&stream);
    EXPECT_FALSE(msg.valid());
    
    // no header tail
    stream.clear();
    stream.str("HTTP/1.1 200 OK\r\n");
    msg = message::from_stream(&stream);
    EXPECT_FALSE(msg.valid());
    
    // embeded linefeed
    stream.clear();
    stream.str("HTTP/1.1\n200 OK\r\n\r\n");
    msg = message::from_stream(&stream);
    EXPECT_FALSE(msg.valid());
    
    // embeded carriage return
    stream.clear();
    stream.str("HTTP/1.1\r200 OK\r\n\r\n");
    msg = message::from_stream(&stream);
    EXPECT_FALSE(msg.valid());
    
    // - LIMITS -
    
    stream.clear();
    stream.str(get_test_request());
    msg = message::from_stream(&stream, &opts);
    EXPECT_EQ(msg.start_line(), "GET /hello.txt HTTP/1.1");
    EXPECT_EQ(msg.header_fields().size(), 3);
    EXPECT_TRUE(msg.valid());
    
    // line size limit
    opts.line_size_limit = 20; // default: 8000
    stream.clear();
    stream.str("GET /long/path/to/a/resource HTTP/1.1\r\n\r\n");
    msg = message::from_stream(&stream, &opts);
    EXPECT_FALSE(msg.valid());
    
    // header count limit
    opts.line_size_limit = 8000; // reset
    opts.header_count_limit = 4; // default: 101
    stream.clear();
    stream.str(get_test_response());
    msg = message::from_stream(&stream, &opts);
    EXPECT_FALSE(msg.valid());
    
    // body size limit
    opts.header_count_limit = 4;
    opts.body_size_limit = 20; // default: 200000
    stream.clear();
    stream.str(get_test_response());
    msg = message::from_stream(&stream, &opts);
    EXPECT_FALSE(msg.valid());
}
