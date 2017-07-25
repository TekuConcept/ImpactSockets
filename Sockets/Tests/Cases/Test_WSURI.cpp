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