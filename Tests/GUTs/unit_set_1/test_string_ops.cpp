/**
 * Created by TekuConcept on January 4, 2018
 */

#include <gtest/gtest.h>
#include <utils/string_ops.h>

using namespace impact;

TEST(test_string_ops, replace_all)
{
    std::string value, select, change;
    
    value  = "foo bar baz bar qux";
    select = "bar";
    change = ":";
    internal::replace_all(&value, &select, &change);
    EXPECT_EQ(value, "foo : baz : qux");
    
    value  = "foo bar baz bar qux";
    select = "bar ";
    internal::replace_all(&value, &select, NULL);
    EXPECT_EQ(value, "foo baz qux");
    
    internal::replace_all(&value, NULL, NULL);
    EXPECT_EQ(value, "foo baz qux");
    
    internal::replace_all(NULL, NULL, NULL);
}


TEST(test_string_ops, trim_whitespace)
{
    std::string value;
    
    value = "\t foo ";
    internal::trim_whitespace(&value);
    EXPECT_EQ(value, "foo");
    
    value = "bar\t";
    internal::trim_whitespace(&value);
    EXPECT_EQ(value, "bar");
    
    internal::trim_whitespace(NULL);
}


TEST(test_string_ops, split)
{
    std::string value, delimiter;
    std::vector<std::string> tokens;
    
    value     = "foo, bar, baz, qux";
    delimiter = ", ";
    
    tokens = internal::split(NULL, NULL);
    EXPECT_EQ(tokens.size(), 0UL);
    
    tokens = internal::split(&value, NULL);
    ASSERT_EQ(tokens.size(), 1UL);
    EXPECT_EQ(tokens[0], value);
    
    tokens = internal::split(&value, &delimiter);
    ASSERT_EQ(tokens.size(), 4UL);
    EXPECT_EQ(tokens[0], "foo");
    EXPECT_EQ(tokens[1], "bar");
    EXPECT_EQ(tokens[2], "baz");
    EXPECT_EQ(tokens[3], "qux");
    
    tokens = internal::split(&value, ", ");
    EXPECT_EQ(tokens.size(), 4UL);
    
    value = " foo \t bar baz     qux ";
    std::regex expression("[\t ]+");
    tokens = internal::split(&value, expression);
    ASSERT_EQ(tokens.size(), 4UL);
    
    value = "foo;bar\\;baz;qux";
    tokens = internal::split(&value, ';', '\\');
    ASSERT_EQ(tokens.size(), 3UL);
    EXPECT_EQ(tokens[0], "foo");
    EXPECT_EQ(tokens[1], "bar\\;baz");
    EXPECT_EQ(tokens[2], "qux");
}
