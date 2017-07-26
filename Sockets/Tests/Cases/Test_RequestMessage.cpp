/**
 * Created by TekuConcept on July 24, 2017
 */

#include <gtest/gtest.h>
#include <RFC/2616>

using namespace Impact;

TEST(TestRequestMessage, SimpleRequestMessage) {
    RFC2616::RequestMessage message1(
        RFC2616::Request::METHOD::GET,
        "/path/to/resource?query"
    );
    EXPECT_EQ(message1.toString(),
        "GET /path/to/resource?query HTTP/1.1\r\n\r\n");
    
    RFC2616::RequestMessage message2(
        RFC2616::Request::METHOD::OPTIONS,
        "*"
    );
    EXPECT_EQ(message2.toString(), "OPTIONS * HTTP/1.1\r\n\r\n");
    
    RFC2616::RequestMessage message3(
        RFC2616::Request::METHOD::GET,
        ""
    );
    EXPECT_EQ(message3.toString(), "GET / HTTP/1.1\r\n\r\n");
}

TEST(TestRequestMessage, AverageRequestMessage) {
    RFC2616::RequestMessage message(RFC2616::Request::METHOD::GET, "");
    message.addHeader(RFC2616::HEADER::Host, "www.example.com");
    message.addUserHeader("MyHeader", "myValue");
    EXPECT_EQ(message.toString(),
        "GET / HTTP/1.1\r\nHost: www.example.com\r\nMyHeader: myValue\r\n\r\n");
}

TEST(TestRequestMessage, HTTPRequestParse) {
    std::stringstream request("GET / HTTP/1.1\r\n\r\n");
    bool check = false;
    RFC2616::RequestMessage message =
        RFC2616::RequestMessage::tryParse(request, check);
    ASSERT_TRUE(check);

    EXPECT_EQ(message.method(), RFC2616::Request::METHOD::GET);
    EXPECT_EQ(message.resource(), "/");
    EXPECT_EQ(message.major(), 1);
    EXPECT_EQ(message.minor(), 1);
}