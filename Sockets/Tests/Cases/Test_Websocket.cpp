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

#ifndef DMSG
#define DMSG(x) std::cerr << x << std::endl
#endif
void serializeOut(DataFrame frame, std::stringstream &_stream_) {
    _stream_ << (unsigned char)((frame.finished ? 0x80 : 0x0) |
                               ((frame.reserved & 0x7) << 4)  | 
                                (frame.opcode   & 0xF));
    
    unsigned char l;
    if(frame.length <= 125)
         l = (unsigned char)(frame.length);
    else if (frame.length <= 65535)
         l = (unsigned char)126;
    else l = (unsigned char)127;
    _stream_ << (unsigned char)((frame.masked << 7) | l);
    
    if(frame.length >= 126 && frame.length <= 65535) {
        _stream_ << (char)(frame.length >> 8);
        _stream_ << (char)(frame.length & 0xFF);
    }
    else if (frame.length > 65535) {
        for(unsigned short i = 8; i <= 64; i+=8)
            _stream_ << (char)((frame.length>>(64-i)) & 0xFF);
    }
    
    unsigned char maskKey[4];
    if(frame.masked) {
        maskKey[0] = '\x37';
        maskKey[1] = '\xFA';
        maskKey[2] = '\x21';
        maskKey[3] = '\x3D';
    }
    for(unsigned short i = 0; i < 4; i++) {
        if(frame.masked) {
            // generate mask key
            _stream_ << maskKey[i];
        }
        else maskKey[i] = '\0';
    }
    for(uint64_t i = 0; i < frame.length; i++)
        _stream_ << (char)(frame.data[i]^maskKey[i%4]);
}

DataFrame serializeIn(std::stringstream &_stream_) {
    DataFrame frame;
    unsigned char c;
    
    _stream_ >> c;
    frame.finished = ((c & 0x80) == 0x80);
    frame.reserved = ((c >> 4) & 0x7);
    frame.opcode   = (c & 0xF);
    
    _stream_ >> c;
    frame.masked         = ((c & 0x80) == 0x80);
    unsigned char length = (c & 0x7F);
    if(length <= 125)
        frame.length     = length;
    else if(length == 126) {
        frame.length     = (_stream_.get() << 8) | _stream_.get();
    }
    else {
        for(unsigned short i = 0; i < 8; i++) {
            frame.length <<= 8;
            frame.length |= _stream_.get();
        }
    }
    
    std::string data(frame.length, '\0');
    unsigned char maskKey[4];
    for(unsigned short i = 0; i < 4; i++) {
        if(frame.masked) maskKey[i] = (char)_stream_.get();
        else             maskKey[i] = '\0';
    }
    for(uint64_t i = 0; i < frame.length; i++)
        data[i] = (char)(_stream_.get()^maskKey[i%4]);
    frame.data = data.c_str();
    
    return frame;
}

TEST(TestWebsocket, serialize) {
    std::stringstream ssClient, ssServer;
    WSURI uri("ws://localhost:8080/");
    WebsocketClient client(ssClient, uri);
    WebsocketServer server(ssServer);
    
    server.sendText("Hello");
    client.sendText("Hello");
    
    std::string dataClient = ssClient.str();
    ASSERT_EQ(dataClient.length(), 11);
    unsigned char compare2[] = "\x81\x85";
    for(unsigned int i = 0; i < 2; i++) {
        // can't compare mask and masked data because
        // mask is randomly generated every call
        EXPECT_EQ(dataClient[i], compare2[i]);
    }
    
    std::string dataServer = ssServer.str();
    ASSERT_EQ(dataServer.length(), 7);
    unsigned char compare[] = "\x81\x05\x48\x65\x6C\x6C\x6F";
    for(unsigned int i = 0; i < dataServer.length(); i++) {
        EXPECT_EQ(dataServer[i], compare[i]);
    }
    
    // TODO: figure out how to setup and test received frames
    std::stringstream ss3(dataServer);
    DataFrame frame3 = serializeIn(ss3);
    EXPECT_TRUE(frame3.finished);
    EXPECT_EQ(frame3.reserved, 0);
    EXPECT_EQ(frame3.opcode, 1);
    EXPECT_FALSE(frame3.masked);
    EXPECT_EQ(frame3.length, 5);
    std::string line3(frame3.data);
    EXPECT_EQ(line3, "Hello");
    
    std::stringstream ss4(dataClient);
    DataFrame frame4 = serializeIn(ss4);
    EXPECT_TRUE(frame4.finished);
    EXPECT_EQ(frame4.reserved, 0);
    EXPECT_EQ(frame4.opcode, 1);
    EXPECT_TRUE(frame4.masked);
    EXPECT_EQ(frame4.length, 5);
    std::string line4(frame4.data);
    EXPECT_EQ(line4, "Hello");
}