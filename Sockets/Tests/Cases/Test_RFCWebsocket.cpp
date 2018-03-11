/**
 * Created by TekuConcept on July 27, 2017
 */

#include <gtest/gtest.h>
#include <RFC/2616>
#include <RFC/4648>
#include <RFC/6455>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace Impact;
using namespace RFC6455;

TEST(TestRFCWebsocket, create) {
    URI uri("ws://localhost:8080/path?query");
    std::stringstream tcpStream;
    WebsocketClientNode client(tcpStream, uri);
    WebsocketServerNode server(tcpStream);
    SUCCEED();
}

TEST(TestRFCWebsocket, initiateClientHandshake) {
    std::stringstream tcpStream;
    
    URI uri("ws://localhost:8080/path?query");
    WebsocketClientNode client(tcpStream, uri);
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

TEST(TestRFCWebsocket, initiateServerHandshake) {
    std::stringstream tcpStream;
    tcpStream << "GET /path?query HTTP/1.1\r\n";
    tcpStream << "Connection: upgrade\r\n";
    tcpStream << "Upgrade: websocket\r\n";
    tcpStream << "Host: localhost:8080\r\n";
    tcpStream << "sec-websocket-key: Tv/QGMqG8C28PDHiyMST3g==\r\n";
    tcpStream << "sec-websocket-version: 13\r\n";
    tcpStream << "\r\n";
    
    WebsocketServerNode server(tcpStream);
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

TEST(TestRFCWebsocket, acceptHandshake) {
    std::stringstream tcpStream;
    
    URI uri("ws://localhost:8080/path?query");
    WebsocketClientNode client(tcpStream, uri);
    client.initiateHandshake();
    
    WebsocketServerNode server(tcpStream);
    ASSERT_TRUE(server.initiateHandshake());
    ASSERT_TRUE(client.acceptHandshake());
}

TEST(TestRFCWebsocket, state) {
    std::stringstream tcpStream;
    URI uri("ws://localhost:8080/");
    
    WebsocketClientNode client(tcpStream, uri);
    WebsocketServerNode server(tcpStream);
    EXPECT_EQ(client.getState(), RFC6455::STATE::CLOSED);
    EXPECT_EQ(server.getState(), RFC6455::STATE::CLOSED);
    
    EXPECT_TRUE(client.initiateHandshake());
    EXPECT_EQ(client.getState(), RFC6455::STATE::CONNECTING);
    
    EXPECT_TRUE(server.initiateHandshake());
    EXPECT_EQ(server.getState(), RFC6455::STATE::OPEN);
    
    EXPECT_TRUE(client.acceptHandshake());
    EXPECT_EQ(client.getState(), RFC6455::STATE::OPEN);
    
    client.close();
    server.read();
    EXPECT_EQ(client.getState(), RFC6455::STATE::CLOSED);
    //EXPECT_EQ(server.getState(), RFC6455::STATE::CLOSED);
}

TEST(TestRFCWebsocket, serializeOut) {
    std::stringstream tcpStream;
    URI uri("ws://localhost:8080/");
    WebsocketClientNode client(tcpStream, uri);
    WebsocketServerNode server(tcpStream);
    
    EXPECT_TRUE(client.initiateHandshake());
    EXPECT_TRUE(server.initiateHandshake());
    EXPECT_TRUE(client.acceptHandshake());
    
    tcpStream.str(std::string());
    client.sendText("Hello");
    std::string dataClient = tcpStream.str();
    ASSERT_EQ(dataClient.length(), 11);
    unsigned char compare2[] = "\x81\x85";
    for(unsigned int i = 0; i < 2; i++) {
        // can't compare mask and masked data because
        // mask is randomly generated every call
        char a = dataClient[i], b = compare2[i];
        EXPECT_EQ(a, b);
    }
    
    tcpStream.str(std::string());
    server.sendText("Hello");
    std::string dataServer = tcpStream.str();
    ASSERT_EQ(dataServer.length(), 7);
    unsigned char compare[] = "\x81\x05\x48\x65\x6C\x6C\x6F";
    for(unsigned int i = 0; i < dataServer.length(); i++) {
        char a = dataServer[i], b = compare[i];
        EXPECT_EQ(a, b);
    }
}

#ifndef DMSG
#define DMSG(x) std::cerr << x << std::endl
#endif

// TODO: Make direct unit test for this function instead of indirect
DataFrame serializeIn(std::iostream& _stream_) {
    #define BAD_FRAME_TEST(c) if(!(_stream_ >> c)) return frame;
    DataFrame frame;
    frame.bad = true;
    char byte;
    
    BAD_FRAME_TEST(byte)
    frame.finished = ((byte & 0x80) == 0x80);
    frame.reserved = ((byte >> 4) & 0x7);
    frame.opcode   = (byte & 0xF);
    
    BAD_FRAME_TEST(byte)
    frame.masked         = ((byte & 0x80) == 0x80);
    unsigned char length = (byte & 0x7F);
    if(length <= 125)
        frame.data.resize(length, '\0');
    else if(length == 126) {
        char msb, lsb;
        BAD_FRAME_TEST(msb)
        BAD_FRAME_TEST(lsb)
        frame.data.resize((msb << 8) | lsb, '\0');
    }
    else {
		// 64-bit messages not supported yet
        unsigned long long int pad = 0;
        for(unsigned short i = 0; i < 8; i++) {
            BAD_FRAME_TEST(byte);
            pad = (pad << 8) | byte;
        }
        frame.data.resize((unsigned int)pad, '\0');
    }
    
    unsigned char maskKey[4];
    for(unsigned short i = 0; i < 4; i++) {
        if(frame.masked) {
            BAD_FRAME_TEST(byte);
            maskKey[i] = byte;
        }
        else maskKey[i] = '\0';
    }
	for (uint64_t i = 0; i < frame.data.length(); i++) {
		// limited array size; for 64-bit sizes, try queued processing
		BAD_FRAME_TEST(byte);
		frame.data[(const unsigned int)i] = (char)((int)(0xFF&byte) ^ maskKey[i%4]);
	}
    
    frame.bad = false;
    return frame;
    #undef BAD_FRAME_TEST
}

TEST(TestRFCWebsocket, serializeIn) {
    // TODO: figure out how to setup and test received frames
    std::stringstream ss3("\x81\x05\x48\x65\x6C\x6C\x6F");
    DataFrame frame3 = serializeIn(ss3);
    EXPECT_TRUE(frame3.finished);
    EXPECT_EQ(frame3.reserved, 0);
    EXPECT_EQ(frame3.opcode, 1);
    EXPECT_FALSE(frame3.masked);
    EXPECT_EQ(frame3.data.length(), 5);
    EXPECT_EQ(frame3.data, "Hello");
    
    std::stringstream ss4("\x81\x85\x37\xfa\x21\x3d\x7f\x9f\x4d\x51\x58");
    DataFrame frame4 = serializeIn(ss4);
    EXPECT_TRUE(frame4.finished);
    EXPECT_EQ(frame4.reserved, 0);
    EXPECT_EQ(frame4.opcode, 1);
    EXPECT_TRUE(frame4.masked);
    EXPECT_EQ(frame4.data.length(), 5);
    EXPECT_EQ(frame4.data, "Hello");
}

TEST(TestRFCWebsocket, badFrame) {
    std::string binary = "\x81\x05\x48\x65\x6C\x6C\x6F";
    std::stringstream tcpStream(binary);
    DataFrame frame = serializeIn(tcpStream);
    EXPECT_FALSE(frame.bad);
    
    for(unsigned int i = 0; i < binary.length(); i++) {
        tcpStream.clear();
        tcpStream.str(binary.substr(0,i));
        frame = serializeIn(tcpStream);
        EXPECT_TRUE(frame.bad);
    }
    
    binary = "\x81\x85\x37\xfa\x21\x3d\x7f\x9f\x4d\x51\x58";
    tcpStream.clear();
    tcpStream.str(binary);
    frame = serializeIn(tcpStream);
    EXPECT_FALSE(frame.bad);
    
    for(unsigned int i = 0; i < binary.length(); i++) {
        tcpStream.clear();
        tcpStream.str(binary.substr(0,i));
        frame = serializeIn(tcpStream);
        EXPECT_TRUE(frame.bad);
    }
}