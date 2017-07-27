/**
 * Created by TekuConcept on July 22, 2017
 */

#include <gtest/gtest.h>
#include <RFC/2616>

using namespace Impact;

TEST(TestConst2616, HTTPStatusCodes) {
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

TEST(TestConst2616, HTTPRequestMethod) {
    EXPECT_EQ(RFC2616::toString(RFC2616::METHOD::CONNECT), "CONNECT");
}

TEST(TestConst2616, HTTPHeaderCodes) {
    EXPECT_EQ(RFC2616::toString(RFC2616::HEADER::Allow), "Allow");
    EXPECT_EQ(RFC2616::toString(RFC2616::HEADER::AcceptRanges),
        "Accept-Ranges");
    EXPECT_EQ(RFC2616::toString(RFC2616::HEADER::Accept), "Accept");
    EXPECT_EQ(RFC2616::toString(RFC2616::HEADER::CacheControl),
        "Cache-Control");
}

TEST(TestConst2616, toLower) {
    std::string test   = "abcDEF123-+/";
    std::string expect = "abcdef123-+/";
    for(unsigned int i = 0; i < test.length(); i++) {
        EXPECT_EQ(RFC2616::toLower(test[i]), expect[i]);
    }
}

TEST(TestConst2616, toUpper) {
    std::string test   = "abcDEF123-+/";
    std::string expect = "ABCDEF123-+/";
    for(unsigned int i = 0; i < test.length(); i++) {
        EXPECT_EQ(RFC2616::toUpper(test[i]), expect[i]);
    }
}

TEST(TestConst2616, isWhiteSpace) {
    std::string test = " \ta";
    EXPECT_TRUE( RFC2616::isWhiteSpace(test[0]));
    EXPECT_TRUE( RFC2616::isWhiteSpace(test[1]));
    EXPECT_FALSE(RFC2616::isWhiteSpace(test[2]));
}

TEST(TestConst2616, validStatusCode) {
    EXPECT_TRUE( RFC2616::validStatusCode(200));
    EXPECT_FALSE(RFC2616::validStatusCode(250));
}

TEST(TestConst2616, findHeader) {
    std::string header1 = "CONNECTION";
    std::string header2 = "Header-Not-Recognized";
    
    RFC2616::HEADER hID1;
    ASSERT_TRUE(RFC2616::findHeader(header1, hID1));
    EXPECT_EQ(hID1, RFC2616::HEADER::Connection);
    
    RFC2616::HEADER hID2;
    ASSERT_FALSE(RFC2616::findHeader(header2, hID2));
}