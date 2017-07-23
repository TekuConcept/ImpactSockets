/**
 * Created by TekuConcept on July 21, 2017
 */

#include <gtest/gtest.h>
#include <string>
#include <RFC2616.h>
#include <RFC6455.h>

using namespace Impact;

//
// Section 3: WebSocket URIs
//

TEST(TestRFCStandard, URIPrefix) {
    ASSERT_EQ(RFC6455::URI::getProtocol(), "ws");
}

TEST(TestRFCStandard, URISecurePrefix) {
    ASSERT_EQ(RFC6455::URI::getSecureProtocol(), "wss");
}

TEST(TestRFCStandard, URIPort) {
    ASSERT_EQ(RFC6455::URI::getDefaultPort(), 80);
}

TEST(TestRFCStandard, URISecurePort) {
    ASSERT_EQ(RFC6455::URI::getDefaultSecurePort(), 443);
}

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
    EXPECT_TRUE(RFC6455::URI::validate("ws://192.168.0.2"));
    EXPECT_TRUE(RFC6455::URI::validate("wss://www.example.com/"));
    EXPECT_FALSE(RFC6455::URI::validate("rtp://a.z"));
    EXPECT_FALSE(RFC6455::URI::validate("ws://-a.io"));
    
    EXPECT_TRUE(RFC6455::URI::validate("ws://127.0.0.1:80"));
    EXPECT_TRUE(RFC6455::URI::validate("wss://localhost:943/"));
    EXPECT_TRUE(RFC6455::URI::validate("ws://a.z:"));
    EXPECT_FALSE(RFC6455::URI::validate("ws://a.z:90223"));
    
    RFC6455::URI::Info info0;
    std::string uri1 = "ws://www.example.com:8080/path#fragment";
    ASSERT_TRUE(RFC6455::URI::parse(uri1, info0));
    EXPECT_EQ(info0.host, "www.example.com");
    EXPECT_EQ(info0.port, 8080);
    EXPECT_EQ(info0.secure, false);
    EXPECT_EQ(info0.resourceName, "/path");
    
    RFC6455::URI::Info info1;
    std::string uri2 = "wss://a.z";
    ASSERT_TRUE(RFC6455::URI::parse(uri2, info1));
    EXPECT_EQ(info1.host, "a.z");
    EXPECT_EQ(info1.port, 443);
    EXPECT_EQ(info1.secure, true);
    EXPECT_EQ(info1.resourceName, "");
}

TEST(TestRFCStandard, GetRequestHeaders) {
    RFC6455::URI::Info info;
    if(!RFC6455::URI::parse("ws://localhost:8080/path?query", info))
        FAIL();
    std::string header = RFC6455::getRequestHeader(info);
    
    // must be a valid http request
    EXPECT_TRUE(RFC2616::Request::validate(header));
    
    // must be a get request
    EXPECT_EQ(header.find("GET"), 0);
    
    // must be at least http version 1.1
    EXPECT_NE(header.find("HTTP/1.1"), std::string::npos);
    
    // request-uri must match resource name
    EXPECT_NE(header.find(info.resourceName), std::string::npos);
    
    // must contain host header field (optional port when not default)
    // must contain upgrade header with value "websocket"
    // must contain connection header with value "upgrade"
    // must contain Sec-WebSocket-Key header with value of random 16B base64
    // (Sec-WebSocket-Key must be random for each new connection)
    // may contain Origin header (non web browsers)
    // must contain Sec-WebSocket-Version header with value of 13
    // may contain Sec-WebSocket-Protocol header
    // may contain Sec-WebSocket-Extensions header
    // may contain other header fields
}