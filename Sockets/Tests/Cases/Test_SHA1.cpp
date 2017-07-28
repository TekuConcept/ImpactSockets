/**
 * Created by TekuConcept on July 28, 2017
 */

#include <gtest/gtest.h>
#include <RFC/3174>

using namespace Impact;

TEST(TestSHA1, digest) {
    std::string test = "hello world";
    EXPECT_EQ(SHA1::digest(test),
"\x2a\xae\x6c\x35\xc9\x4f\xcf\xb4\x15\xdb\xe9\x5f\x40\x8b\x9c\xe9\x1e\xe8\x46\xed");
}