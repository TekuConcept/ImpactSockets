/**
 * Created by TekuConcept on July 22, 2017
 */

#include <gtest/gtest.h>
#include <RFC/2616>

using namespace Impact;

TEST(TestRFCStandard, HTTPStatusCodes) {
    EXPECT_EQ(RFC2616::STATUS::CONTINUE, 100);
    EXPECT_EQ(RFC2616::STATUS::OK, 200);
    EXPECT_EQ(RFC2616::STATUS::MULTIPLE_CHOICES, 300);
    EXPECT_EQ(RFC2616::STATUS::BAD_REQUEST, 400);
    EXPECT_EQ(RFC2616::STATUS::INTERNAL_SERVER_ERROR, 500);
    
    EXPECT_EQ(RFC2616::toString(RFC2616::STATUS::CONTINUE), "CONTINUE");
    EXPECT_EQ(RFC2616::toString(RFC2616::STATUS::OK), "OK");
    EXPECT_EQ(RFC2616::toString(RFC2616::STATUS::MULTIPLE_CHOICES),
        "MULTIPLE CHOICES");
    EXPECT_EQ(RFC2616::toString(RFC2616::STATUS::BAD_REQUEST),
        "BAD REQUEST");
    EXPECT_EQ(RFC2616::toString(RFC2616::STATUS::INTERNAL_SERVER_ERROR),
        "INTERNAL SERVER ERROR");
}

TEST(TestRFCStandard, HTTPRequestMethod) {
    EXPECT_EQ(RFC2616::toString(RFC2616::Request::METHOD::CONNECT), "CONNECT");
}

TEST(TestRFCStandard, HTTPHeaderCodes) {
    EXPECT_EQ(RFC2616::toString(RFC2616::HEADER::Allow), "Allow");
    EXPECT_EQ(RFC2616::toString(RFC2616::HEADER::AcceptRanges),
        "Accept-Ranges");
    EXPECT_EQ(RFC2616::toString(RFC2616::HEADER::Accept), "Accept");
    EXPECT_EQ(RFC2616::toString(RFC2616::HEADER::CacheControl),
        "Cache-Control");
}

TEST(TestRFCStandard, HTTPRequestParse) {
    std::stringstream request("GET / HTTP/1.1\r\n\r\n");
    RFC2616::Request::Info info;
    ASSERT_TRUE(RFC2616::Request::parse(request, info));
    EXPECT_EQ(info.method, RFC2616::Request::METHOD::GET);
    EXPECT_EQ(info.requestURI, "/");
    EXPECT_EQ(info.major, 1);
    EXPECT_EQ(info.minor, 1);
}