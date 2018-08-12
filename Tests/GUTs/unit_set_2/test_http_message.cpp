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
    start-line   = request-line / status-line
    request-line = method SP request-target SP HTTP-version CRLF
    status-line  = HTTP-version SP status-code SP reason-phrase CRLF
    */
    std::stringstream stream(get_test_response());
    message msg;
    
    // returns blank message
    msg = message::from_stream(NULL);
    
    // message from non-null stream
    msg = message::from_stream(&stream);
    EXPECT_EQ(msg.start_line(), "HTTP/1.1 200 OK");
    EXPECT_EQ(msg.header_fields().size(), 8);
    EXPECT_EQ(msg.message_body(),
        "Hello World! My payload includes a trailing CRLF.\r\n");
}
