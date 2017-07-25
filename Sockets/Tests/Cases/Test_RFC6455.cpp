/**
 * Created by TekuConcept on July 21, 2017
 */

#include <gtest/gtest.h>
#include <string>
#include <RFC/2616>
#include <RFC/6455>

using namespace Impact;

//
// Section 3: WebSocket URIs
//

TEST(TestRFCStandard, URIDerefFragment) {
    std::string uri = "ws://www.example.com/path#fragment";
    RFC6455::URI::DerefFragment(uri);
    ASSERT_EQ(uri, "ws://www.example.com/path");
}

TEST(TestRFCStandard, URIEscapeAllPound) {
    std::string uri = "ws://localhost:80/path#with#symbols";
    RFC6455::URI::EscapeAllPound(uri);
    ASSERT_EQ(uri, "ws://localhost:80/path%%23with%%23symbols");
}

//
// Section 4.1: Client Requirements
//

// while connecting, socket will initially be in CONNECTING state

TEST(TestRFCStandard, URIValidInfo) {
    EXPECT_TRUE(RFC6455::WSURI::validate("ws://192.168.0.2"));
    EXPECT_TRUE(RFC6455::WSURI::validate("wss://www.example.com/"));
    EXPECT_FALSE(RFC6455::WSURI::validate("rtp://a.z"));
    EXPECT_FALSE(RFC6455::WSURI::validate("ws://-a.io"));
    
    EXPECT_TRUE(RFC6455::WSURI::validate("ws://127.0.0.1:80"));
    EXPECT_TRUE(RFC6455::WSURI::validate("wss://localhost:943/"));
    EXPECT_TRUE(RFC6455::WSURI::validate("ws://a.z:"));
    EXPECT_FALSE(RFC6455::WSURI::validate("ws://a.z:90223"));
    
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
    EXPECT_EQ(uri2.resource(), "");
}

TEST(TestRFCStandard, GetRequestMessage) {
    bool check = false;
    RFC6455::WSURI uri = RFC6455::WSURI::tryParse(
        "ws://localhost:8080/path?query", check);
    ASSERT_TRUE(check);
    std::string header = RFC6455::getRequestMessage(uri);
    
    // must be a valid http request
    EXPECT_TRUE(RFC2616::RequestMessage::validate(header));
    
    // must be a get request
    EXPECT_EQ(header.find("GET"), 0);
    
    // must be at least http version 1.1
    EXPECT_NE(header.find("HTTP/1.1"), std::string::npos);
    
    // request-uri must match resource name
    EXPECT_NE(header.find(uri.resource()), std::string::npos);
    
    // must contain host header field (optional port when not default)
    EXPECT_NE(header.find("Host: localhost:8080"), std::string::npos);
    
    // must contain upgrade header with value "websocket"
    EXPECT_NE(header.find("Upgrade: websocket"), std::string::npos);
    
    // must contain connection header with value "upgrade"
    EXPECT_NE(header.find("Connection: upgrade"), std::string::npos);
    
    // must contain Sec-WebSocket-Key header with value of random 16B base64
    // (Sec-WebSocket-Key must be random for each new connection)
    // may contain Origin header (non web browsers)
    // must contain Sec-WebSocket-Version header with value of 13
    // may contain Sec-WebSocket-Protocol header
    // may contain Sec-WebSocket-Extensions header
    // may contain other header fields
}