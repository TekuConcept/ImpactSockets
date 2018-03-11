/**
 * Created by TekuConcept on March 10, 2018
 */

#include <gtest/gtest.h>
#include "GTestStream.h"

#include <iostream>

#include <RFC/StringCodec.h>

using namespace Impact;

TEST(TestStringCodec, charData) {
    const char data[3] = {'\x00','\x7F','\xFF'};
    std::string utf8;
    ASSERT_TRUE(StringCodec::encodeUTF8(data,3,utf8));
    ASSERT_EQ(utf8.length(),4);
    EXPECT_EQ((unsigned int)(0xFF&utf8[0]),0x00);
    EXPECT_EQ((unsigned int)(0xFF&utf8[1]),0x7F);
    EXPECT_EQ((unsigned int)(0xFF&utf8[2]),0xC3);
    EXPECT_EQ((unsigned int)(0xFF&utf8[3]),0xBF);
}

TEST(TestStringCodec, intData) {
    const unsigned int data[] = {
        0x00000000,
        0x0000007F,
        0x000007FF,
        0x0000FFFF,
        0x0010FFFF
    };
    const unsigned int test[] = {
        0x00,
        0x7F,
        0xDF,0xBF,
        0xEF,0xBF,0xBF,
        0xF4,0x8F,0xBF,0xBF
    };
    std::string utf8;
    ASSERT_TRUE(StringCodec::encodeUTF8(data,5,utf8));
    ASSERT_EQ(utf8.length(),11);
    for(auto i = 0; i < 11; i++) {
        TEST_COUT << "Test " << i << ": ";
        EXPECT_EQ((unsigned int)(0xFF&utf8[i]), test[i]);
        std::cout << "Pass!" << std::endl;
    }
    
    const unsigned int test_fail[] = { 0x110000 };
    ASSERT_FALSE(StringCodec::encodeUTF8(test_fail,1,utf8));
}

