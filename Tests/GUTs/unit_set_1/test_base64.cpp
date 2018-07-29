/**
 * Created by TekuConcept on July 27, 2017
 */

#include <iostream>

#include <gtest/gtest.h>
#include <rfc/base64.h>

using namespace impact;

TEST(test_base64, encode) {
    std::vector<std::string> input{
        "",
        "f",
        "fo",
        "foo",
        "foob",
        "fooba",
        "foobar"
    };
    std::vector<std::string> expected{
        "",
        "Zg==",
        "Zm8=",
        "Zm9v",
        "Zm9vYg==",
        "Zm9vYmE=",
        "Zm9vYmFy"
    };
    
    for (size_t i = 0; i < input.size(); i++) {
        std::string result;
        EXPECT_TRUE(base64::encode(input[i], &result));
        EXPECT_EQ(result, expected[i]);
    }
    
    std::string message =
        "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10";
    std::string data;
    EXPECT_TRUE(base64::encode(message,&data));
    EXPECT_EQ(data, "AQIDBAUGBwgJCgsMDQ4PEA==");
    
    message = "\xb3\x7a\x4f\x2c\xc0\x62\x4f\x16\x90\xf6\x46\x06\xcf\x38\x59\x45\xb2\xbe\xc4\xea";
    EXPECT_TRUE(base64::encode(message,&data));
    EXPECT_EQ(data, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");
}

TEST(test_base64, decode) {
    std::vector<std::string> input{
        "",
        "Zg==",
        "Zm8=",
        "Zm9v",
        "Zm9vYg==",
        "Zm9vYmE=",
        "Zm9vYmFy"
    };
    std::vector<std::string> expected{
        "",
        "f",
        "fo",
        "foo",
        "foob",
        "fooba",
        "foobar"
    };
    
    for (size_t i = 0; i < input.size(); i++) {
        std::string result;
        EXPECT_TRUE(base64::decode(input[i], &result));
        EXPECT_EQ(result, expected[i]);
    }
    
    std::string data = "AQIDBAUGBwgJCgsMDQ4PEA==";
    std::string message;
    EXPECT_TRUE(base64::decode(data, &message));
    EXPECT_EQ(message,
        "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10");
    
    std::string test;
    EXPECT_FALSE(base64::decode("illegal.chars", &test));
}