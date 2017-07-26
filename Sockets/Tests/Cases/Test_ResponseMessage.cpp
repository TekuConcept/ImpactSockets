/**
 * Create by TekuConcept on July 25, 2017
 */

#include <gtest/gtest.h>
#include <RFC/2616>

using namespace Impact;

TEST(TestResponseMessage, SimpleResponseMessage) {
    RFC2616::ResponseMessage message(RFC2616::STATUS::OK);
    EXPECT_EQ(message.toString(), "HTTP/1.1 200 OK\r\n\r\n");
}

TEST(TestResponseMessage, AverageResponseMessage) {
    RFC2616::ResponseMessage message(RFC2616::STATUS::OK);
    message.addHeader(RFC2616::HEADER::Connection, "Closed");
    message.addUserHeader("MyHeader", "myValue");
    EXPECT_EQ(message.toString(),
        "HTTP/1.1 200 OK\r\nConnection: Closed\r\nMyHeader: myValue\r\n\r\n");
}

TEST(TestResponseMessage, HTTPResponseParse) {
    std::stringstream request("HTTP/1.1 200 OK\r\n\r\n");
    bool check = false;
    RFC2616::ResponseMessage message =
        RFC2616::ResponseMessage::tryParse(request, check);
    ASSERT_TRUE(check);

    EXPECT_EQ(message.status(), RFC2616::STATUS::OK);
    EXPECT_EQ(message.major(), 1);
    EXPECT_EQ(message.minor(), 1);
}