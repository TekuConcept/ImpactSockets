/**
 * Created by TekuConcept on July 21, 2017
 */

#include <gtest/gtest.h>
#include <string>
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
    std::string uri;
    RFC6455::URI::Info info;
    
    // std::string uri0 = "ws://192.168.0.2";
    // EXPECT_TRUE(RFC6455::URI::generateInfo(uri0, info));
    // std::string uri1 = "wss://www.example.com/";
    // EXPECT_TRUE(RFC6455::URI::generateInfo(uri1, info));
    // std::string uri2 = "rtp://a.z";
    // EXPECT_FALSE(RFC6455::URI::generateInfo(uri2, info));
    // std::string uri3 = "ws://-a.io";
    // EXPECT_FALSE(RFC6455::URI::generateInfo(uri3, info));
    
    std::string uri4 = "ws://127.0.0.1:80";
    EXPECT_TRUE(RFC6455::URI::generateInfo(uri4, info));
    // std::string uri5 = "wss://localhost:943/";
    // EXPECT_TRUE(RFC6455::URI::generateInfo(uri5, info));
    // std::string uri6 = "ws://a.z:";
    // EXPECT_FALSE(RFC6455::URI::generateInfo(uri6, info));
    // std::string uri7 = "ws://a.z:90223";
    // EXPECT_FALSE(RFC6455::URI::generateInfo(uri7, info));
    
    // RFC6455::URI::Info info0;
    // std::string uri8 = "ws://www.example.com:8080/path#fragment";
    // EXPECT_TRUE(RFC6455::URI::generateInfo(uri8, info0));
    // EXPECT_EQ(info0.host, "www.example.com");
    // EXPECT_EQ(info0.port, 8080);
    // EXPECT_EQ(info0.secure, false);
    // EXPECT_EQ(info0.resourceName, "/path");
    
    // RFC6455::URI::Info info1;
    // std::string uri9 = "wss://a.z";
    // EXPECT_TRUE(RFC6455::URI::generateInfo(uri9, info1));
    // EXPECT_EQ(info1.host, "a.z");
    // EXPECT_EQ(info1.port, 443);
    // EXPECT_EQ(info1.secure, true);
    // EXPECT_EQ(info1.resourceName, "");
}
