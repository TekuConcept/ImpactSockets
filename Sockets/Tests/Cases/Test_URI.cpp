/**
 * Created by TekuConcept on July 25, 2017
 */

#include <gtest/gtest.h>
#include <RFC/2616>

using namespace Impact;
using namespace RFC2616;

TEST(TestURI, Create) {
    try {
        URI uri("ws://www.example.com/");
        SUCCEED();
    } catch(std::exception) {
        FAIL();
    }
    
    bool check;
    URI uri = URI::tryParse("http://www.example.com", check);
    EXPECT_TRUE(check);
}

TEST(TestURI, Scheme) {
    URI uri1("http://example.com/");
    EXPECT_EQ(uri1.scheme(), "http");
    
    bool check = false;
    URI uri2 = URI::tryParse("http:", check);
    EXPECT_FALSE(check);
    
    URI uri3("HTTP://EXAMPLE.COM/");
    EXPECT_EQ(uri3.scheme(), "http");
}

TEST(TestURI, HostIPv6) {
    EXPECT_FALSE(URI::validate("http://"));
    EXPECT_FALSE(URI::validate("http://["));
    EXPECT_FALSE(URI::validate("http://[:"));
    EXPECT_FALSE(URI::validate("http://[]"));
    EXPECT_FALSE(URI::validate("http://[:]"));
    EXPECT_FALSE(URI::validate("http://[0:0:0:0:0:0:0:0:0]"));
    EXPECT_TRUE(URI::validate("http://[::]"));
    EXPECT_FALSE(URI::validate("http://[:/:]"));
    EXPECT_FALSE(URI::validate("http://[:R:0G]"));
    EXPECT_FALSE(URI::validate("http://[0123::ABCDE::]"));
    
    bool check = false;
    URI uri = URI::tryParse(
        "http://[2001:0db8:85A3::8A2E:03f0:7334]/path", check);
    ASSERT_TRUE(check);
    EXPECT_EQ(uri.host(), "[2001:0db8:85a3::8a2e:03f0:7334]");
}

TEST(TestURI, Host) {
    EXPECT_TRUE(URI::validate("http://192.168.0.2"));
    EXPECT_TRUE(URI::validate("http://www.example.com/"));
    EXPECT_FALSE(URI::validate("http://-a.io"));
    EXPECT_TRUE(URI::validate("http://a.z:"));
    EXPECT_FALSE(URI::validate("http://"));
    EXPECT_TRUE(URI::validate("http://a"));
}

TEST(TestURI, Port) {
    EXPECT_TRUE(URI::validate("http://127.0.0.1:80"));
    EXPECT_TRUE(URI::validate("http://localhost:943/"));
    EXPECT_FALSE(URI::validate("http://a.z:90223"));
    EXPECT_FALSE(URI::validate("http://a.z:100001"));
    
    URI uri1("http://localhost");
    EXPECT_EQ(uri1.port(), 80);
    
    URI uri2("https://localhost");
    EXPECT_EQ(uri2.port(), 443);
    
    URI uri3("http://localhost:5020");
    EXPECT_EQ(uri3.port(), 5020);
    
    URI uri4("rtp://localhost");
    EXPECT_EQ(uri4.port(), 0);
    
    URI uri5("ws://localhost");
    EXPECT_EQ(uri5.port(), 80);
    
    URI uri6("wss://localhost");
    EXPECT_EQ(uri6.port(), 443);
}

TEST(TestURI, Secure) {
    URI uri1("http://www.example.com/");
    EXPECT_FALSE(uri1.secure());
    
    URI uri2("https://www.example.com/");
    EXPECT_TRUE(uri2.secure());
    
    URI uri3("https://www.example.com:8080/");
    EXPECT_TRUE(uri3.secure());
    
    URI uri4("ws://www.example.com/");
    EXPECT_FALSE(uri4.secure());
    
    URI uri5("wss://www.example.com/");
    EXPECT_TRUE(uri5.secure());
    
    URI uri6("unkown://www.example.com/");
    EXPECT_FALSE(uri6.secure());
}

TEST(TestURI, Resource) {
    URI uri1("http://www.example.com/path/to/something");
    EXPECT_EQ(uri1.resource(), "/path/to/something");
    
    URI uri2("http://127.0.0.1:80/path/to?query");
    EXPECT_EQ(uri2.resource(), "/path/to?query");
    
    URI uri3("http://localhost");
    EXPECT_EQ(uri3.resource(), "/");
    
    URI uri4("http://www.example.com/path/with#fragment");
    EXPECT_EQ(uri4.resource(), "/path/with");
    
    URI uri5("http://www.example.com/path with spaces/");
    EXPECT_EQ(uri5.resource(), "/path%%20with%%20spaces/");
}