/**
 * Created by TekuConcept on July 27, 2017
 */

#include <gtest/gtest.h>
#include <RFC/6455>

using namespace Impact;
using namespace RFC6455;

TEST(TestConst6455, findHeader) {
    std::string header1 = "Sec-WebSocket-Key";
    std::string header2 = "Something-Else";
    
    HEADER hID1;
    EXPECT_TRUE(findHeader(header1, hID1));
    EXPECT_EQ(hID1, HEADER::SecWebSocketKey);
    
    HEADER hID2;
    EXPECT_FALSE(findHeader(header2, hID2));
}

TEST(TestConst6455, toString) {
    EXPECT_EQ(toString(HEADER::SecWebSocketKey), "Sec-WebSocket-Key");
}