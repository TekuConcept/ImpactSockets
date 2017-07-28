/**
 * Created by TekuConcept on July 27, 2017
 */

#include <gtest/gtest.h>
#include <RFC/4648>
#include <iostream>

using namespace Impact;
using namespace RFC4648;

TEST(TestBase64, encode) {
    EXPECT_EQ(Base64::encode(""), "");
    EXPECT_EQ(Base64::encode("f"), "Zg==");
    EXPECT_EQ(Base64::encode("fo"), "Zm8=");
    EXPECT_EQ(Base64::encode("foo"), "Zm9v");
    EXPECT_EQ(Base64::encode("foob"), "Zm9vYg==");
    EXPECT_EQ(Base64::encode("fooba"), "Zm9vYmE=");
    EXPECT_EQ(Base64::encode("foobar"), "Zm9vYmFy");
    
    std::string message =
        "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10";
    std::string data = Base64::encode(message);
    EXPECT_EQ(data, "AQIDBAUGBwgJCgsMDQ4PEA==");
}

TEST(TestBase64, decode) {
    EXPECT_EQ(Base64::decode(""), "");
    EXPECT_EQ(Base64::decode("Zg=="), "f");
    EXPECT_EQ(Base64::decode("Zm8="), "fo");
    EXPECT_EQ(Base64::decode("Zm9v"), "foo");
    EXPECT_EQ(Base64::decode("Zm9vYg=="), "foob");
    EXPECT_EQ(Base64::decode("Zm9vYmE="), "fooba");
    EXPECT_EQ(Base64::decode("Zm9vYmFy"), "foobar");
    
    bool check = false;
    std::string data = "AQIDBAUGBwgJCgsMDQ4PEA==";
    std::string message = Base64::decode(data, check);
    ASSERT_TRUE(check);
    EXPECT_EQ(message,
        "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10");
    
    bool check2 = false;
    std::string test = Base64::decode("illegal.chars", check2);
    EXPECT_FALSE(check2);
}