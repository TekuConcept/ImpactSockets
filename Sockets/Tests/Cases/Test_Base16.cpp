/**
 * Created by TekuConcept on July 28, 2017
 */

#include <gtest/gtest.h>
#include <RFC/4648>
#include <iostream>

using namespace Impact;

TEST(TestBase16, encode) {
    EXPECT_EQ(Base16::encode(""), "");
    EXPECT_EQ(Base16::encode("f"), "66");
    EXPECT_EQ(Base16::encode("fo"), "666F");
    EXPECT_EQ(Base16::encode("foo"), "666F6F");
    
    std::string message =
        "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10";
    std::string data = Base16::encode(message);
    EXPECT_EQ(data, "0102030405060708090A0B0C0D0E0F10");
}

TEST(TestBase16, decode) {
    EXPECT_EQ(Base16::decode(""), "");
    EXPECT_EQ(Base16::decode("66"), "f");
    EXPECT_EQ(Base16::decode("666F"), "fo");
    EXPECT_EQ(Base16::decode("666F6F"), "foo");
    EXPECT_EQ(Base16::decode("666f6f"), "foo");
    
    bool check = false;
    std::string data = "0102030405060708090A0B0C0D0E0F10";
    std::string message = Base16::decode(data, check);
    ASSERT_TRUE(check);
    EXPECT_EQ(message,
        "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10");
    
    bool check2 = false;
    std::string test = Base16::decode("illegal.chars", check2);
    EXPECT_FALSE(check2);
}