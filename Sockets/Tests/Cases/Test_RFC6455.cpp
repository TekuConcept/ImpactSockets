/**
 * Created by TekuConcept on July 21, 2017
 */

#include <gtest/gtest.h>
#include <string>
#include <RFC/2616>
#include <RFC/6455>

using namespace Impact;

//
// Section 4.1: Client Requirements
//

// while connecting, socket will initially be in CONNECTING state

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