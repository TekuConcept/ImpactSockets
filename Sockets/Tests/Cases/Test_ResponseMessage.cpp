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
    message.addHeader("MyHeader", "myValue");
    EXPECT_EQ(message.toString(),
        "HTTP/1.1 200 OK\r\nConnection: Closed\r\nMyHeader: myValue\r\n\r\n");
}

TEST(TestResponseMessage, ResponseParse) {
    using RFC2616::ResponseMessage;
    std::stringstream response("HTTP/1.1 200 OK\r\n\r\n");
    bool check = false;
    ResponseMessage message = ResponseMessage::tryParse(response, check);
    ASSERT_TRUE(check);

    EXPECT_EQ(message.status(), RFC2616::STATUS::OK);
    EXPECT_EQ(message.major(), 1);
    EXPECT_EQ(message.minor(), 1);
    
    response.clear();
    response.str("HTTP/1.1 20"); // message timed out or disconnect
    ResponseMessage::tryParse(response, check);
    ASSERT_FALSE(check);
}