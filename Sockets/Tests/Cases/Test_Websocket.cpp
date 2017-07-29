/**
 * Created by TekuConcept on July 27, 2017
 */

#include <gtest/gtest.h>
#include <RFC/2616>
#include <RFC/4648>
#include <RFC/6455>
#include <sstream>

using namespace Impact;
using namespace RFC6455;

TEST(TestWebsocket, create) {
    WSURI uri("ws://localhost:8080/path?query");
    std::stringstream tcpStream;
    WebsocketClient client(tcpStream, uri);
    WebsocketServer server(tcpStream);
    SUCCEED();
}

TEST(TestWebsocket, initiateClientHandshake) {
    std::stringstream tcpStream;
    
    WSURI uri("ws://localhost:8080/path?query");
    WebsocketClient client(tcpStream, uri);
    client.initiateHandshake();

    // must be a valid http request
    bool check = false;
    RFC2616::RequestMessage message =
        RFC2616::RequestMessage::tryParse(tcpStream, check);
    ASSERT_TRUE(check);
    
    // must be a get request
    EXPECT_EQ(message.method(), RFC2616::METHOD::GET);

    // must be at least http version 1.1
    EXPECT_GE(message.major(), 1);
    EXPECT_GE(message.minor(), 1);

    // request-uri must match resource name
    EXPECT_EQ(message.resource(), uri.resource());
    
    // must contain host header field (optional port when not default)
    RFC2616::string host("localhost:8080");
    EXPECT_EQ(message.getHeaderValue(RFC2616::HEADER::Host), host);
    
    // must contain upgrade header with value "websocket"
    RFC2616::string upgrade("websocket");
    EXPECT_EQ(message.getHeaderValue(RFC2616::HEADER::Upgrade), upgrade);
    
    // must contain connection header with value "upgrade"
    RFC2616::string connection("upgrade");
    EXPECT_EQ(message.getHeaderValue(RFC2616::HEADER::Connection), connection);
    
    // must contain Sec-WebSocket-Key header with value of random 16B base64
    std::string base64 = message.getHeaderValue(
        RFC6455::toString(RFC6455::HEADER::SecWebSocketKey)
    );
    // 16 byte base64-encoded string is always 24 chars long and ends with '=='
    // (Sec-WebSocket-Key must be random for each new connection)
    EXPECT_EQ(base64.length(), 24);
    EXPECT_NE(base64.find("=="), std::string::npos);

    // must contain Sec-WebSocket-Version header with value of 13
    EXPECT_EQ(message.getHeaderValue(
        RFC6455::toString(RFC6455::HEADER::SecWebSocketVersion)), "13");
    
    // may contain Origin header (non web browsers)
    // may contain Sec-WebSocket-Protocol header
    // may contain Sec-WebSocket-Extensions header
    // may contain other header fields
}

TEST(TestWebsocket, initiateServerHandshake) {
    std::stringstream tcpStream;
    tcpStream << "GET /path?query HTTP/1.1\r\n";
    tcpStream << "Connection: upgrade\r\n";
    tcpStream << "Upgrade: websocket\r\n";
    tcpStream << "Host: localhost:8080\r\n";
    tcpStream << "sec-websocket-key: Tv/QGMqG8C28PDHiyMST3g==\r\n";
    tcpStream << "sec-websocket-version: 13\r\n";
    tcpStream << "\r\n";
    
    WebsocketServer server(tcpStream);
    ASSERT_TRUE(server.initiateHandshake());
    
    bool check = false;
    RFC2616::ResponseMessage message =
        RFC2616::ResponseMessage::tryParse(tcpStream, check);
    ASSERT_TRUE(check);
    
    EXPECT_EQ(message.status(), RFC2616::STATUS::SWITCHING);
    EXPECT_GE(message.major(), 1);
    EXPECT_GE(message.minor(), 1);
    
    RFC2616::string upgrade("websocket");
    EXPECT_EQ(message.getHeaderValue(RFC2616::HEADER::Upgrade), upgrade);
    
    RFC2616::string connection("upgrade");
    EXPECT_EQ(message.getHeaderValue(RFC2616::HEADER::Connection), connection);
    
    std::string hash = message.getHeaderValue(
        RFC6455::toString(RFC6455::HEADER::SecWebSocketAccept));
    EXPECT_EQ(hash.length(), 28);
    bool check2;
    std::string md = Base64::decode(hash, check2);
    EXPECT_TRUE(check2);
}

TEST(TestWebsocket, acceptResponse) {
    std::stringstream tcpStream;
    
    WSURI uri("ws://localhost:8080/path?query");
    WebsocketClient client(tcpStream, uri);
    client.initiateHandshake();
    
    WebsocketServer server(tcpStream);
    ASSERT_TRUE(server.initiateHandshake());
    ASSERT_TRUE(client.acceptResponse());
}