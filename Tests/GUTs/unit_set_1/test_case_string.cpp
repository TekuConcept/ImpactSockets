/**
 * Created by TekuConcept on December 17, 2018
 */

#include <string>
#include <sstream>

#include <gtest/gtest.h>
#include <utils/case_string.h>

TEST(test_case_string, equality)
{
    impact::case_string stringA = "Hello World!";
    std::string stringB = "hello world!";
    std::string stringC = "foo bar baz!";
    
    EXPECT_TRUE(stringA == stringB);
    EXPECT_TRUE(stringB == stringA);
    EXPECT_TRUE(stringA != stringC);
    EXPECT_TRUE(stringC != stringA);
    
    EXPECT_TRUE(stringA == "hello world!");
    EXPECT_TRUE("hello world!" == stringA);
}


TEST(test_case_string, streams)
{
    std::stringstream ss;
    impact::case_string stringA = "Hello World!";
    impact::case_string stringB;
    
    ss << stringA;
    EXPECT_EQ(ss.str(), stringA);
    
    ss >> stringB;
    EXPECT_EQ(stringB, "Hello");
}


#include <iostream>
TEST(test_case_string, swap)
{
    impact::case_string A = "Hello World";
    std::string B = "Goodbye World";
    
    impact::swap(A, B);
    EXPECT_EQ(A, "Goodbye World");
    EXPECT_EQ(B, "Hello World");
}


TEST(test_case_string, move)
{
    impact::case_string A = "Hello World";
    std::string B = impact::move(A);
    
    EXPECT_EQ(B, "Hello World");
}


TEST(test_case_string, compare)
{
    impact::case_string a = "HELLO WORLD";
    EXPECT_EQ(a.compare(0, 5, "hello"), 0);
}
