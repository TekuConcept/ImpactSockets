/**
 * Created by TekuConcept on July 27, 2017
 */

#include <gtest/gtest.h>
#include <RFC/4648>
#include <iostream>

using namespace Impact;
using namespace RFC4648;

TEST(TestBase64, encode) {
    std::string message =
        "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10";
    std::string data = Base64::encode(message);
    EXPECT_EQ(data, "AQIDBAUGBwgJCgsMDQ4PEA==");
}

TEST(TestBase64, decode) {
    SUCCEED();
}