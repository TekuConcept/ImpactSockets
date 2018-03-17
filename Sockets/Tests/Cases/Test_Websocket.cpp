/**
 * Created by TekuConcept on March 8, 2018
 */

#include <gtest/gtest.h>
#include "GTestStream.h"

#include <sstream>
#include <thread>
#include <vector>

#include <RFC/2616> // URI
#include <RFC/4648> // Base64
#include <RFC/6455>
#include <Websocket.h>
#include <wsmanip>
#include <Internal/WebsocketUtils.h>

using namespace Impact;
using namespace Internal;

URI uri("ws://localhost:8080/path?query");

TEST(TestWebsocket, create) {
    std::stringstream tcpStream;
    Websocket client(tcpStream,uri,WS_TYPE::WS_CLIENT);
    Websocket server(tcpStream,uri,WS_TYPE::WS_SERVER);
    SUCCEED();
}

TEST(TestWebsocket, modes) {
    std::stringstream stream;
    Websocket socket(stream,uri,WS_TYPE::WS_CLIENT);
    
    // default output mode
    EXPECT_EQ(socket.out_mode(), WS_MODE::TEXT);
    
    socket.out_mode(WS_MODE::BINARY);
    EXPECT_EQ(socket.out_mode(), WS_MODE::BINARY);
    socket << ws::text;
    EXPECT_EQ(socket.out_mode(), WS_MODE::TEXT);
    socket << ws::binary;
    EXPECT_EQ(socket.out_mode(), WS_MODE::BINARY);
    
    // default input mode
    EXPECT_EQ(socket.in_mode(), WS_MODE::TEXT);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 * Test the underlying serializer protocols used in websocket streaming      *
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

TEST(TestWebsocket, writeHeader) {
    #define RESET \
        stream.clear();\
        stream.str("");\
        randomEngine.seed(0)
    std::stringstream stream;
    std::mt19937 randomEngine;
    WSFrameContext header;
    std::string result;
    std::vector<std::string> tests = {
        std::string("\x00\x00", 2),
        std::string("\x80\x00", 2),
        std::string("\x70\x00", 2),
        std::string("\x0F\x00", 2),
        std::string("\x00\x80\x8C\x97\xB7\xD8", 6),
        std::string("\x00\x7D", 2),
        std::string("\x00\x7E\xFF\xFF", 4),
        std::string("\x00\x7F\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 10),
        std::string("\x00\xFE\xFF\xFF\x8C\x97\xB7\xD8", 8)
    };
    
    // blank frame (superposition-isolation)
    header.finished = false;
    header.reserved = 0;
    header.opcode   = 0;
    header.masked   = false;
    header.length   = 0;
    for(int i = 0; i < 4; i++)
        header.mask_key[0] = 0;
    
    // tests
    for(unsigned int i = 0; i < tests.size(); i++) {
        TEST_COUT << "Test " << i << ": ";
        switch(i) {
        case 1: header.finished = true;                          break;
        case 2: header.finished = false; header.reserved = 0xFF; break;
        case 3: header.reserved = 0x00;  header.opcode   = 0xFF; break;
        case 4: header.opcode   = 0x00;  header.masked   = true; break;
        case 5: header.masked   = false; header.length   = 125;  break;
        case 6: header.length   = 0xFFFF;                        break;
        case 7: header.length   = 0xFFFFFFFFFFFFFFFF;            break;
        case 8: header.length   = 0xFFFF;header.masked   = true; break;
        default: break; // 0-frame
        }
        
        RESET;
        ASSERT_TRUE(WebsocketUtils::writeHeader(stream, header, randomEngine));
        result = stream.str();
        EXPECT_EQ(result.length(), tests[i].length());
        EXPECT_EQ(result, tests[i]);
        std::cout << "Pass!" << std::endl;
    }
    
    RESET;
    header.length = 0;
    header.masked = true;
    ASSERT_TRUE(WebsocketUtils::writeHeader(stream, header, randomEngine));
    result = stream.str();
    const char* key = "\x8C\x97\xB7\xD8";
    for(int i = 0; i < 4; i++) {
        EXPECT_EQ((int)(result[i+2]&0xFF), (int)((unsigned char)key[i]&0xFF));
    }
    
    #undef RESET
}

TEST(TestWebsocket, writeData) {
    #define RESET \
        stream.clear();\
        stream.str("");\
        keyOffset = 0
    std::stringstream stream;
    std::string message = "Hello World!", result;
    WSFrameContext context;
    int keyOffset;
    
    //
    // writeData only needs 'mask_key' from the context
    //
    
    RESET;
    
    //
    // write unmasked data
    //
    context.masked   = false;
    for(auto i = 0; i < 4; i++)
        context.mask_key[i] = 0;
    ASSERT_TRUE(WebsocketUtils::writeData(stream, context,
        message.c_str(), message.length(), keyOffset));
    result = stream.str();
    EXPECT_EQ(result, message);

    RESET;

    //
    // write masked data
    //
    context.masked = true;
    context.mask_key[0] = 0x8C;
    context.mask_key[1] = 0x97;
    context.mask_key[2] = 0xB7;
    context.mask_key[3] = 0xD8;
    
    ASSERT_TRUE(WebsocketUtils::writeData(stream, context,
        message.c_str(), message.length(), keyOffset));
    result = stream.str();
    std::string scrambled(
        "\xC4\xF2\xDB\xB4\xE3\xB7\xE0\xB7\xFE\xFB\xD3\xF9", 12);
    EXPECT_EQ(result, scrambled);

    #undef RESET
}

TEST(TestWebsocket, readHeader) {
    #define RESET(x) \
        stream.clear();\
        stream.str(x)
    std::stringstream stream;
    WSFrameContext header;
    std::vector<std::string> tests = {
        "",
        std::string("\x00", 1),
        std::string("\x00\x00", 2),
        std::string("\x80\x00", 2),
        std::string("\x70\x00", 2),
        std::string("\x0F\x00", 2),
        std::string("\x00\x80\x8C\x97\xB7\xD8", 6),
        std::string("\x00\x7D", 2),
        std::string("\x00\x7E\xFF\xFF", 4),
        std::string("\x00\x7F\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 10),
        std::string("\x00\xFE\xFF\xFF\x8C\x97\xB7\xD8", 8)
    };
    unsigned long long int expected[][9] = {
        {0x0,0x0,0x0,0x0,0x0000000000000000,0x00,0x00,0x00,0x00},
        {0x1,0x0,0x0,0x0,0x0000000000000000,0x00,0x00,0x00,0x00},
        {0x0,0x7,0x0,0x0,0x0000000000000000,0x00,0x00,0x00,0x00},
        {0x0,0x0,0xF,0x0,0x0000000000000000,0x00,0x00,0x00,0x00},
        {0x0,0x0,0x0,0x1,0x0000000000000000,0x8C,0x97,0xB7,0xD8},
        {0x0,0x0,0x0,0x0,0x000000000000007D,0x00,0x00,0x00,0x00},
        {0x0,0x0,0x0,0x0,0x000000000000FFFF,0x00,0x00,0x00,0x00},
        {0x0,0x0,0x0,0x0,0xFFFFFFFFFFFFFFFF,0x00,0x00,0x00,0x00},
        {0x0,0x0,0x0,0x1,0x000000000000FFFF,0x8C,0x97,0xB7,0xD8}
    };
    
    RESET(tests[0]);
    ASSERT_FALSE(WebsocketUtils::readHeader(stream,header));
    
    RESET(tests[1]);
    ASSERT_FALSE(WebsocketUtils::readHeader(stream,header));
    
    for(int i = 0; i < 9; i++) {
        TEST_COUT << "Test " << i << ": ";
        
        RESET(tests[i+2]);
        ASSERT_TRUE(WebsocketUtils::readHeader(stream,header));
        EXPECT_EQ((int)header.finished, expected[i][0]);
        EXPECT_EQ(     header.reserved, expected[i][1]);
        EXPECT_EQ(     header.opcode,   expected[i][2]);
        EXPECT_EQ((int)header.masked,   expected[i][3]);
        EXPECT_EQ(     header.length,   expected[i][4]);
        for(int j = 0; j < 4; j++)
            EXPECT_EQ(header.mask_key[j], expected[i][5+j]);
        
        std::cout << "Pass!" << std::endl;
    }
    
    #undef RESET
}

TEST(TestWebsocket, readData) {
    std::stringstream stream;
    WSFrameContext context;
    int keyOffset = 0;
    std::string result, message = "Hello World!", data;
    data.resize(message.length(),'\0');
    
    //
    // readData only needs 'mask_key' from the context
    //
    
    //
    // read unmasked data
    //
    context.masked = false;
    for(auto i = 0; i < 4; i++)
        context.mask_key[i] = 0;
    stream.clear();
    stream.str(message);
    EXPECT_EQ(
        WebsocketUtils::readData(
            stream,context,&data[0],data.length(),keyOffset),
        data.length()
    );
    EXPECT_EQ(data, message);
    
    //
    // limited read
    //
    stream.clear();
    stream.str("yes");
    keyOffset = 0;
    EXPECT_EQ(
        WebsocketUtils::readData(
            stream,context,&data[0],data.length(),keyOffset),
        3
    );
    
    //
    // read masked data
    //
    context.masked = true;
    context.mask_key[0] = 0x8C;
    context.mask_key[1] = 0x97;
    context.mask_key[2] = 0xB7;
    context.mask_key[3] = 0xD8;
    stream.clear();
    std::string scrambled(
        "\xC4\xF2\xDB\xB4\xE3\xB7\xE0\xB7\xFE\xFB\xD3\xF9", 12);
    stream.str(scrambled);
    for(unsigned int i = 0; i < data.length(); i++) data[0] = '\0';
    keyOffset = 0;
    EXPECT_EQ(
        WebsocketUtils::readData(
            stream,context,&data[0],data.length(),keyOffset),
        12
    );
    EXPECT_EQ(data, message);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 * Test the underlying messaging protocol used in websocket.shakeHands();    *
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

TEST(TestWebsocket, syn_handshake) {
    using RFC2616::RequestMessage;
    
    URI uri("ws://localhost:8080/path?query");
    std::stringstream tcpStream;
    std::mt19937 randomEngine;
    randomEngine.seed(0);
    
    auto key = WebsocketUtils::SYN(tcpStream, uri, randomEngine);
    ASSERT_TRUE(key.length() > 0);
    
    // must be a valid http request
    bool check = false;
    RequestMessage message = RequestMessage::tryParse(tcpStream, check);
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

TEST(TestWebsocket, synack_handshake) {
    std::stringstream tcpStream;
    tcpStream << "GET /path?query HTTP/1.1\r\n";
    tcpStream << "Connection: upgrade\r\n";
    tcpStream << "Upgrade: websocket\r\n";
    tcpStream << "Host: localhost:8080\r\n";
    tcpStream << "sec-websocket-key: Tv/QGMqG8C28PDHiyMST3g==\r\n";
    tcpStream << "sec-websocket-version: 13\r\n";
    tcpStream << "\r\n";
    
    std::string key = WebsocketUtils::SYNACK(tcpStream);
    ASSERT_TRUE(key.length() > 0);
    
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
    
    // non-standard port tested; assuming standard ports work the same
    // key size tested; assuming correct hashed key is returned
    
    tcpStream.clear();
    tcpStream.str("");
    tcpStream << "GET /path?query HTTP/1.1\r\n";
    tcpStream << "Connection: upgrade\r\n";
    tcpStream << "Upgrade: websocket\r\n";
    tcpStream << "Host: local"; // request interrupted
    auto badKey = WebsocketUtils::SYNACK(tcpStream);
    ASSERT_TRUE(badKey.length() <= 0);
}

TEST(TestWebsocket, ack_handshake) {
    URI uri("ws://localhost:8080/path?query");
    std::stringstream tcpStream;
    std::mt19937 randomEngine;
    randomEngine.seed(0);
    
    auto key1 = WebsocketUtils::SYN(tcpStream,uri,randomEngine);
    ASSERT_TRUE(key1.length() > 0);
    auto key2 = WebsocketUtils::SYNACK(tcpStream);
    ASSERT_TRUE(key2.length() > 0);
    EXPECT_EQ(key1, key2);
    
    ASSERT_TRUE(WebsocketUtils::ACK(tcpStream, key1));
}