// /**
//  * Created by TekuConcept on August 9, 2018
//  */

// #include <gtest/gtest.h>
// #include <rfc/uri.h>
// #include <rfc/http/uri.h>

// using namespace impact;

// TEST(test_http_uri, generic) {
//     http::uri u;
    
//     // Base URI class parsing
//     EXPECT_TRUE(uri::parse( "http://www.example.com/", &u));
//     // invalid scheme
//     EXPECT_FALSE(uri::parse( "foo://www.example.com/", &u));
//     // empty host
//     EXPECT_FALSE(uri::parse("http:///foo/bar", &u));
//     // user info present
//     EXPECT_FALSE(uri::parse("http://user:info@www.example.com/", &u));
    
//     // Derived URI class parsing
//     EXPECT_TRUE(http::uri::parse("http://www.example.com/", NULL));
//     // invalid scheme
//     EXPECT_FALSE(http::uri::parse("foo://www.example.com/", NULL));
//     // empty host
//     EXPECT_FALSE(http::uri::parse("http://user:info@www.example.com/", NULL));
// }


// TEST(test_http_uri, assignment) {
//     uri u;
    
//     // copy assign invalid uri
//     EXPECT_TRUE(uri::parse("foo://localhost/", &u));
//     try {
//         http::uri u3(u);
//         (void)u3;
//         FAIL();
//     }
//     catch (...) {}
    
//     // copy assign valid uri
//     EXPECT_TRUE(uri::parse("http://localhost/", &u));
//     try {
//         http::uri u3(u);
//         (void)u3;
//     }
//     catch (...) { FAIL(); }
// }
