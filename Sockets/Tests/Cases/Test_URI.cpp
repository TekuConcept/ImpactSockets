/**
 * Created by TekuConcept on July 25, 2017
 */

#include <gtest/gtest.h>
#include <RFC/2616>

using namespace Impact;
using namespace RFC2616;

TEST(TestURI, Scheme) {
    URI uri1("http://example.com/");
    EXPECT_EQ(uri1.scheme(), "http");
}

TEST(TestURI, HostIPv6) {
    EXPECT_TRUE(URI::validate("http://[2001:0db8:85A3::8a2e:0370:7334]/path"));
    EXPECT_TRUE(URI::validate("http://[::]"));
    EXPECT_FALSE(URI::validate("http://[:]"));
    EXPECT_FALSE(URI::validate("http://[:/:]"));
    EXPECT_FALSE(URI::validate("http://[:R:0G]"));
}

TEST(TestURI, Host) {
    EXPECT_TRUE(URI::validate("http://192.168.0.2"));
    EXPECT_TRUE(URI::validate("http://www.example.com/"));
    EXPECT_FALSE(URI::validate("http://-a.io"));
    EXPECT_TRUE(URI::validate("http://a.z:"));
}

TEST(TestURI, Secure) {
    try {
        URI uri("http://www.example.com/");
        EXPECT_FALSE(uri.secure());
    } catch (std::exception) {
        FAIL();
    }
    
    try {
        URI uri("https://www.example.com/");
        EXPECT_TRUE(uri.secure());
    } catch (std::exception) {
        FAIL();
    }
    
    try {
        URI uri("https://www.example.com:8080/");
        EXPECT_TRUE(uri.secure());
    } catch (std::exception) {
        FAIL();
    }
}