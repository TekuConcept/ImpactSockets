/**
 * 
 */

#include <gtest/gtest.h>
#include <RFC/6455>

using namespace Impact;
using namespace RFC6455;

TEST(TestWebSocketURI, create) {
    try {
        WSURI uri("ws://www.example.com/");
        SUCCEED();
    } catch(std::exception) {
        FAIL();
    }
    
    try {
        bool check;
        WSURI uri = RFC2616::URI::tryParse("http://www.example.com", check);
        FAIL();
    } catch(std::exception) {
        SUCCEED();
    }
}

TEST(TestWebSocketURI, secure) {
    WSURI uri1("ws://www.example.com/");
    EXPECT_FALSE(uri1.secure());
    
    WSURI uri2("wss://www.example.com/");
    EXPECT_TRUE(uri2.secure());
}

TEST(TestWebSocketURI, tryParse) {
    bool check = false;
    WSURI uri = WSURI::tryParse("ws://www.example.com/", check);
    EXPECT_TRUE(check);
}

TEST(TestWebSocketURI, validScheme) {
    try {
        WSURI uri("http://www.example.com/");
        FAIL();
    } catch(std::exception) {
        SUCCEED();
    }
    
    try {
        WSURI uri1("ws://www.example.com/");
        WSURI uri2("wss://www.example.com/");
        SUCCEED();
    } catch(std::exception) {
        FAIL();
    }
}

TEST(TestWebSocketURI, validInfo) {
    EXPECT_TRUE(RFC6455::WSURI::validate("ws://192.168.0.2"));
    EXPECT_TRUE(RFC6455::WSURI::validate("wss://www.example.com/"));
    EXPECT_FALSE(RFC6455::WSURI::validate("rtp://a.z"));
    
    bool check1 = false;
    RFC6455::WSURI uri = RFC6455::WSURI::tryParse(
        "ws://www.example.com:8080/path#fragment", check1);
    ASSERT_TRUE(check1);
    EXPECT_EQ(uri.host(), "www.example.com");
    EXPECT_EQ(uri.port(), 8080);
    EXPECT_EQ(uri.secure(), false);
    EXPECT_EQ(uri.resource(), "/path");
    
    bool check2 = false;
    RFC6455::WSURI uri2 = RFC6455::WSURI::tryParse("wss://a.z", check2);
    ASSERT_TRUE(check2);
    EXPECT_EQ(uri2.host(), "a.z");
    EXPECT_EQ(uri2.port(), 443);
    EXPECT_EQ(uri2.secure(), true);
    EXPECT_EQ(uri2.resource(), "/");
}