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


