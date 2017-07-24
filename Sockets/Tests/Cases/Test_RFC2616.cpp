/**
 * Created by TekuConcept on July 22, 2017
 */

#include <gtest/gtest.h>
#include <RFC2616.h>

using namespace Impact;

TEST(TestRFCStandard, URIScheme) {
    std::string scheme1;
    ASSERT_TRUE(RFC2616::URI::parseScheme("http://example.com/", scheme1));
    EXPECT_EQ(scheme1, "http");
    
    std::string scheme2;
    EXPECT_FALSE(RFC2616::URI::parseScheme("http", scheme2));
}

TEST(TestRFCStandard, URIHostIPv6) {
    EXPECT_TRUE(RFC2616::URI::validate(
        "http://[2001:0db8:85A3::8a2e:0370:7334]/path"));
    EXPECT_TRUE( RFC2616::URI::validate("http://[::]"));
    EXPECT_FALSE(RFC2616::URI::validate("http://[:]"));
    EXPECT_FALSE(RFC2616::URI::validate("http://[:/:]"));
    EXPECT_FALSE(RFC2616::URI::validate("http://[:R:0G]"));
}

TEST(TestRFCStandard, URIHost) {
    EXPECT_TRUE(RFC2616::URI::validate("http://192.168.0.2"));
    EXPECT_TRUE(RFC2616::URI::validate("http://www.example.com/"));
    EXPECT_FALSE(RFC2616::URI::validate("http://-a.io"));
    EXPECT_TRUE(RFC2616::URI::validate("http://a.z:"));
}

TEST(TestRFCStandard, HTTPStatusCodes) {
    EXPECT_EQ(RFC2616::STATUS::CONTINUE, 100);
    EXPECT_EQ(RFC2616::STATUS::OK, 200);
    EXPECT_EQ(RFC2616::STATUS::MULTIPLE_CHOICES, 300);
    EXPECT_EQ(RFC2616::STATUS::BAD_REQUEST, 400);
    EXPECT_EQ(RFC2616::STATUS::INTERNAL_SERVER_ERROR, 500);
    
    EXPECT_EQ(RFC2616::getStatusString(RFC2616::STATUS::CONTINUE), "CONTINUE");
    EXPECT_EQ(RFC2616::getStatusString(RFC2616::STATUS::OK), "OK");
    EXPECT_EQ(RFC2616::getStatusString(RFC2616::STATUS::MULTIPLE_CHOICES),
        "MULTIPLE CHOICES");
    EXPECT_EQ(RFC2616::getStatusString(RFC2616::STATUS::BAD_REQUEST),
        "BAD REQUEST");
    EXPECT_EQ(RFC2616::getStatusString(RFC2616::STATUS::INTERNAL_SERVER_ERROR),
        "INTERNAL SERVER ERROR");
}

TEST(TestRFCStandard, HTTPRequestMethod) {
    EXPECT_EQ(RFC2616::Request::getMethodString(
        RFC2616::Request::METHOD::CONNECT), "CONNECT");
}

TEST(TestRFCStandard, HTTPRequestLine) {
    try {
        std::string line0 = RFC2616::Request::getRequestLine(
            RFC2616::Request::METHOD::GET,
            "/path/to/resource?query"
        );
        EXPECT_EQ(line0, "GET /path/to/resource?query HTTP/1.1\r\n");
    } catch(int e) {
        std::cerr << "Failed valid resource: " << e << std::endl;
        FAIL();
    }
    
    try {
        std::string line1 = RFC2616::Request::getRequestLine(
            RFC2616::Request::METHOD::OPTIONS,
            "*"
        );
        EXPECT_EQ(line1, "OPTIONS * HTTP/1.1\r\n");
    } catch(int e) {
        std::cerr << "Failed valid resource: " << e << std::endl;
        FAIL();
    }
    
    try {
        std::string line2 = RFC2616::Request::getRequestLine(
            RFC2616::Request::METHOD::GET, ""
        );
        std::cerr << "Passed invalid resource" << std::endl;
        FAIL();
    } catch (int) { 
        SUCCEED();
    }
}

TEST(TestRFCStandard, HTTPRequestParse) {
    std::stringstream request("\
GET / HTTP/1.1\r\n\
\r\n\
");
    RFC2616::Request::Info info;
    ASSERT_TRUE(RFC2616::Request::parse(request, info));
    EXPECT_EQ(info.method, RFC2616::Request::METHOD::GET);
    EXPECT_EQ(info.requestURI, "/");
    EXPECT_EQ(info.major, 1);
    EXPECT_EQ(info.minor, 1);
}