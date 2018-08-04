/**
 * Created by TekuConcept on August 1, 2018
 */

#include <gtest/gtest.h>
#include <utils/abnf_ops.h>

using namespace impact;
using namespace internal;

TEST(test_abnf_ops, generic) {
    for (unsigned char c = 'A'; c <= 'Z'; c++)
        EXPECT_TRUE(ALPHA(c));
    for (unsigned char c = 'a'; c <= 'z'; c++)
        EXPECT_TRUE(ALPHA(c));
    EXPECT_FALSE(ALPHA('0'));
    
    EXPECT_TRUE(BIT('0'));
    EXPECT_TRUE(BIT('1'));
    EXPECT_FALSE(BIT('A'));
    
    for (unsigned char c = '\x01'; c < 0x80; c++)
        EXPECT_TRUE(CHAR(c)); // all ascii chars except NULL
    EXPECT_FALSE(CHAR('\x80')); // non-ascii char
    
    EXPECT_TRUE(LF('\n'));
    EXPECT_FALSE(LF('\0'));
    
    EXPECT_TRUE(CR('\r'));
    EXPECT_FALSE(CR('\0'));
    
    EXPECT_TRUE(CRLF('\r', '\n'));
    EXPECT_FALSE(CRLF('\0', '\0'));
    
    EXPECT_TRUE(CTL('\x7F')); // ascii control DEL
    for (unsigned char c = '\0'; c < '\x20'; c++)
        EXPECT_TRUE(CTL(c)); // all ascii control characters
    EXPECT_FALSE(CTL('\x20'));
    
    for (unsigned char c = '0'; c <= '9'; c++)
        EXPECT_TRUE(DIGIT(c));
    EXPECT_FALSE(DIGIT('A'));
    
    EXPECT_TRUE(DQUOTE('\"'));
    EXPECT_FALSE(DQUOTE('\0'));
    
    for (unsigned char c = '0'; c <= '9'; c++)
        EXPECT_TRUE(HEXDIG(c));
    for (unsigned char c = 'A'; c <= 'F'; c++)
        EXPECT_TRUE(HEXDIG(c));
    for (unsigned char c = 'a'; c < 'f'; c++)
        EXPECT_TRUE(HEXDIG(c));
    EXPECT_FALSE(HEXDIG('\0'));
    
    EXPECT_TRUE(HTAB('\t'));
    EXPECT_FALSE(HTAB('\0'));
    
    EXPECT_TRUE(OCTET('\x00'));
    EXPECT_TRUE(OCTET('\xFF'));
    
    EXPECT_TRUE(SP(' '));
    EXPECT_FALSE(SP('\0'));
    
    for (unsigned char c = '\x21'; c <= '\x7E'; c++)
        EXPECT_TRUE(VCHAR(c));
    EXPECT_FALSE(VCHAR('\0'));
    
    EXPECT_TRUE(WSP(' '));
    EXPECT_TRUE(WSP('\t'));
    EXPECT_FALSE(WSP('\0'));
}