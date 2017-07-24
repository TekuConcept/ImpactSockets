/**
 * Created by TekuConcept on July 24, 2017
 */

#include <gtest/gtest.h>
#include <RFC/RequestMessage.h>

using namespace Impact;

TEST(TestRequestMethod, SimpleRequestMessage) {
    RFC2616::Request::Message message1(
        RFC2616::Request::METHOD::GET,
        "/path/to/resource?query"
    );
    EXPECT_EQ(message1.toString(),
        "GET /path/to/resource?query HTTP/1.1\r\n\r\n");
    
    RFC2616::Request::Message message2(
        RFC2616::Request::METHOD::OPTIONS,
        "*"
    );
    EXPECT_EQ(message2.toString(), "OPTIONS * HTTP/1.1\r\n\r\n");
    
    RFC2616::Request::Message message3(
        RFC2616::Request::METHOD::GET,
        ""
    );
    EXPECT_EQ(message3.toString(), "GET / HTTP/1.1\r\n\r\n");
}

TEST(TestRequestMethod, AverageRequestMessage) {
    RFC2616::Request::Message message(RFC2616::Request::METHOD::GET, "");
    message.addHeader(RFC2616::HEADER::Host, "www.example.com");
    EXPECT_EQ(message.toString(),
        "GET / HTTP/1.1\r\nHost: www.example.com\r\n\r\n");
}