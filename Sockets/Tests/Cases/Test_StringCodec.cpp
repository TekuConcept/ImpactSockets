/**
 * Created by TekuConcept on March 10, 2018
 */

#include <gtest/gtest.h>
#include "GTestStream.h"

#include <iostream>

#include <RFC/StringCodec.h>

using namespace Impact;

TEST(TestStringCodec, utf8EncodeCharacters) {
    const char data[3] = {'\x00','\x7F','\xFF'};
    std::string utf8;
    ASSERT_TRUE(StringCodec::encodeUTF8(data,3,utf8));
    ASSERT_EQ(utf8.length(),4);
    EXPECT_EQ((unsigned int)(0xFF&utf8[0]),0x00);
    EXPECT_EQ((unsigned int)(0xFF&utf8[1]),0x7F);
    EXPECT_EQ((unsigned int)(0xFF&utf8[2]),0xC3);
    EXPECT_EQ((unsigned int)(0xFF&utf8[3]),0xBF);
}

TEST(TestStringCodec, utf8EncodePoints) {
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

TEST(TestStringCodec, utf8Decode) {
    int status = UTF8_SUCCESS;
    unsigned int pointCode = 0;
    
    StringCodec::decodeUTF8('\x00',pointCode,status);
    EXPECT_EQ(status, UTF8_SUCCESS);
    EXPECT_EQ(pointCode, 0);
    
    StringCodec::decodeUTF8('\xC0',pointCode,status);
    EXPECT_EQ(status, UTF8_FAIL);
    
    status = UTF8_SUCCESS;
    pointCode = 0;
    StringCodec::decodeUTF8('\xE0',pointCode,status);
    EXPECT_GT(status, UTF8_SUCCESS);
    StringCodec::decodeUTF8('\x89',pointCode,status);
    EXPECT_EQ(status, UTF8_FAIL);
    
    status = UTF8_SUCCESS;
    pointCode = 0;
    StringCodec::decodeUTF8('\xF5',pointCode,status);
    EXPECT_EQ(status, UTF8_FAIL);
    
    status = UTF8_SUCCESS;
    pointCode = 0;
    StringCodec::decodeUTF8('\xF0',pointCode,status);
    EXPECT_GT(status, UTF8_SUCCESS);
    StringCodec::decodeUTF8('\x8F',pointCode,status);
    EXPECT_EQ(status, UTF8_FAIL);
    
    const char* tests[] = {
        // U+0041 U+2262 U+0391 U+002E
        // "A<NOT IDENTICAL TO><ALPHA>."
        "\x41\xE2\x89\xA2\xCE\x91\x2E",
        // U+D55C U+AD6D U+C5B4
        // (Korean "hangugeo", meaning "the Korean language")
        "\xED\x95\x9C\xEA\xB5\xAD\xEC\x96\xB4",
        // U+65E5 U+672C U+8A9E
        // (Japanese "nihongo", meaning "the Japanese language")
        "\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E",
        // UTF8_BOM U+233B4
        // (a Chinese character meaning 'stump of tree')
        "\xEF\xBB\xBF\xF0\xA3\x8E\xB4"
    };
    int length[] = {7,9,9,7};
    unsigned int point[][4] = {
        {0x0041,0x2262,0x0391,0x002E},
        {0xD55C,0xAD6D,0xC5B4,0x0},
        {0x65E5,0x672C,0x8A9E,0x0},
        {UTF8_BOM,0x233B4,0x0,0x0}
    };
    
    for(int i = 0; i < 4; i++) {
        status = UTF8_SUCCESS;
        pointCode = 0;
        for(int j = 0,k=0; j < length[i]; j++) {
            StringCodec::decodeUTF8(tests[i][j],pointCode,status);
            ASSERT_GE(status, UTF8_SUCCESS);
            if(status == UTF8_SUCCESS) {
                EXPECT_EQ(pointCode,point[i][k++]);
                pointCode = 0;
            }
        }
    }
}