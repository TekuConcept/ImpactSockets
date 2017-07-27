/**
 * Created by TekuConcept on July 24, 2017
 */

#include <gtest/gtest.h>
#include <RFC/2616>

#ifndef DMSG
    #include <iostream>
    #define DMSG(x) std::cerr << x << std::endl
#endif

using namespace Impact;

TEST(TestRequestMessage, SimpleRequestMessage) {
    RFC2616::RequestMessage message1(
        RFC2616::METHOD::GET,
        "/path/to/resource?query"
    );
    EXPECT_EQ(message1.toString(),
        "GET /path/to/resource?query HTTP/1.1\r\n\r\n");
    
    RFC2616::RequestMessage message2(
        RFC2616::METHOD::OPTIONS,
        "*"
    );
    EXPECT_EQ(message2.toString(), "OPTIONS * HTTP/1.1\r\n\r\n");
    
    RFC2616::RequestMessage message3(
        RFC2616::METHOD::GET,
        ""
    );
    EXPECT_EQ(message3.toString(), "GET / HTTP/1.1\r\n\r\n");
}

TEST(TestRequestMessage, AverageRequestMessage) {
    RFC2616::RequestMessage message2(RFC2616::METHOD::GET, "");
    EXPECT_TRUE(message2.addHeader(RFC2616::HEADER::Host, "www.example.com"));
    EXPECT_FALSE(message2.addHeader(RFC2616::HEADER::Host, "localhost"));
    EXPECT_TRUE(message2.addHeader("MyHeader", "value1"));
    EXPECT_TRUE(message2.addHeader("MyHeader", "value2"));
    
    RFC2616::RequestMessage message3(RFC2616::METHOD::GET, "");
    message3.addHeader("MyHeader", "value1");
    message3.addHeader("MyHeader", "value2");
    std::string line = message3.toString();
    EXPECT_NE(line.find("MyHeader: value1"), std::string::npos);
    EXPECT_NE(line.find("MyHeader: value2"), std::string::npos);
    
    RFC2616::RequestMessage message(RFC2616::METHOD::GET, "");
    message.addHeader(RFC2616::HEADER::Host, "www.example.com");
    message.addHeader("MyHeader", "myValue");
    EXPECT_EQ(message.toString(),
        "GET / HTTP/1.1\r\nHost: www.example.com\r\nMyHeader: myValue\r\n\r\n");
}

TEST(TestRequestMessage, RequestParse) {
    std::stringstream request("GET / HTTP/1.1\r\n\r\n");
    bool check = false;
    RFC2616::RequestMessage message =
        RFC2616::RequestMessage::tryParse(request, check);
    ASSERT_TRUE(check);

    EXPECT_EQ(message.method(), RFC2616::METHOD::GET);
    EXPECT_EQ(message.resource(), "/");
    EXPECT_EQ(message.major(), 1);
    EXPECT_EQ(message.minor(), 1);
}

TEST(TestMessage, ParseHeaders) {
    std::stringstream request[7];
    bool check[7];
    for(int i = 0; i < 7; i++) check[i] = false;
    
    request[0] << "GET / HTTP/1.1\r\n";
    request[0] << "Host: ImpactSockets\r\n";
    request[0] << "\r\n";
    RFC2616::RequestMessage message0 =
        RFC2616::RequestMessage::tryParse(request[0], check[0]);
    ASSERT_TRUE(check[0]);
    std::string line = message0.toString();
    EXPECT_NE(line.find("Host: ImpactSockets"), std::string::npos);
    
    request[1] << "GET / HTTP/1.1\r\n";
    request[1] << "Entity: \r\n";
    request[1] << "\r\n";
    RFC2616::RequestMessage message1 =
        RFC2616::RequestMessage::tryParse(request[1], check[1]);
    EXPECT_TRUE(check[1]);

    request[2] << "GET / HTTP/1.1\r\n";
    request[2] << ": nothing\r\n";
    request[2] << "\r\n";
    RFC2616::RequestMessage message2 =
        RFC2616::RequestMessage::tryParse(request[2], check[2]);
    EXPECT_FALSE(check[2]);
    
    request[3] << "GET / HTTP/1.1\r\n";
    request[3] << "Malicious : Value\r\n";
    request[3] << "\r\n";
    RFC2616::RequestMessage message3 =
        RFC2616::RequestMessage::tryParse(request[3], check[3]);
    EXPECT_FALSE(check[3]);
    
    // RFC 7230 Section 3.2.2
    // duplicate header values not allowed
    request[4] << "GET / HTTP/1.1\r\n";
    request[4] << "Host: host1\r\n";
    request[4] << "Host: host2\r\n";
    request[4] << "\r\n";
    RFC2616::RequestMessage message4 =
        RFC2616::RequestMessage::tryParse(request[4], check[4]);
    EXPECT_FALSE(check[4]);
    
    // former empty value overridden by new value
    request[5] << "GET / HTTP/1.1\r\n";
    request[5] << "Host:\r\n";
    request[5] << "Host: localhost\r\n";
    request[5] << "\r\n";
    RFC2616::RequestMessage message5 =
        RFC2616::RequestMessage::tryParse(request[5], check[5]);
    EXPECT_TRUE(check[5]);
    
    // Some headers have special exceptions to the duplicate-header rule
    // NOTE: Set-Cookie header is not defined in RFC 2616 standard.
    //       See RFC 6265 for HTTP Cookie and Set-Cookie header fields.
    request[6] << "GET / HTTP/1.1\r\n";
    request[6] << "Set-Cookie: cookie1\r\n";
    request[6] << "Set-Cookie: cookie2\r\n";
    request[6] << "\r\n";
    RFC2616::RequestMessage message6 =
        RFC2616::RequestMessage::tryParse(request[6], check[6]);
    ASSERT_TRUE(check[6]);
}

TEST(TestMessages, GetHeaderValues) {
    std::stringstream request[2];
    
    request[0] << "GET / HTTP/1.1\r\n";
    request[0] << "Host: ImpactSockets\r\n";
    request[0] << "\r\n";
    RFC2616::RequestMessage message1(request[0].str());
    EXPECT_EQ(message1.getHeaderValue(RFC2616::HEADER::Host), "ImpactSockets");
    EXPECT_EQ(message1.getHeaderValue("Host"), "ImpactSockets");
    EXPECT_EQ(message1.getHeaderValue("HOST"), "ImpactSockets");
    EXPECT_EQ(message1.getHeaderValue(RFC2616::HEADER::Date).length(), 0);
    EXPECT_EQ(message1.getHeaderValue("SomeEntity").length(), 0);
    
    request[1] << "GET / HTTP/1.1\r\n";
    request[1] << "SomeEntity: fieldValue\r\n";
    request[1] << "SomeEntity: next value\r\n";
    request[1] << "\r\n";
    RFC2616::RequestMessage message2(request[1].str());
    EXPECT_EQ(message2.getHeaderValue("SomeEntity"),    "fieldValue");
    EXPECT_EQ(message2.getHeaderValue("SOMEENTITY"),    "fieldValue");
    EXPECT_EQ(message2.getHeaderValue("SomeEntity", 0), "fieldValue");
    EXPECT_EQ(message2.getHeaderValue("SomeEntity", 1), "next value");
}
