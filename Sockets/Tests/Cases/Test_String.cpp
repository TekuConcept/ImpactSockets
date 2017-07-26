/**
 * Created by TekuConcept on July 26, 2017
 */

#include <gtest/gtest.h>
#include <RFC/2616>

using namespace Impact;

TEST(TestConst2616, insensitiveStrings) {
    RFC2616::string first  = "HeLlo WorlD!";
    RFC2616::string second = "Hello World!";
    
    EXPECT_EQ(first, second);
    
    std::string third = "Hello World!";
    EXPECT_EQ(first, third);
    EXPECT_EQ(third, first);
}