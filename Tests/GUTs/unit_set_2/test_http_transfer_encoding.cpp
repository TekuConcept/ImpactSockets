/**
 * Created by TekuConcept on December 15, 2018
 */

#include <string>

#include <gtest/gtest.h>
#include "utils/environment.h"
#include "rfc/http/TX/transfer_encoding.h"

using namespace impact;
using namespace http;


#define NO_THROW(code) try { {code} } catch (...) { FAIL(); }
#define THROW(code)    try { {code} FAIL(); } catch (...) { }

TEST(test_http_chunk_ext_token, create)
{
    NO_THROW(
        chunk_ext_token token("foo","bar");
        EXPECT_EQ(token.name(), "foo");
        EXPECT_EQ(token.value(), "bar");
    )
    
    THROW(chunk_ext_token token("/bad", "bar"););
    THROW(chunk_ext_token token("foo", "/bad"););
}


TEST(test_http_transfer_encoding, custom)
{
    NO_THROW( // user defined transfer encoding
        transfer_encoding coding(
            "foobar",
            [](const std::string& data) -> std::string { return data; }
        );
    )
    THROW( // reserved names
        transfer_encoding coding(
            "chunked",
            [](const std::string& data) -> std::string { return data; }
        );
    )
}


TEST(test_http_transfer_encoding, chunked_encoding)
{
    // chunked-body   = *chunk
    //                   last-chunk
    //                   trailer-part
    //                   CRLF
    // chunk          = chunk-size [ chunk-ext ] CRLF
    //                  chunk-data CRLF
    // chunk-size     = 1*HEXDIG
    // last-chunk     = 1*("0") [ chunk-ext ] CRLF
    // chunk-data     = 1*OCTET ; a sequence of chunk-size octets
    // chunk-ext      = *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
    // chunk-ext-name = token
    // chunk-ext-val  = token / quoted-string
    // trailer-part   = *( header-field CRLF )
    
    transfer_encoding_ptr chunked;
    
    // - vanilla -
    chunked = transfer_encoding::chunked();
    EXPECT_EQ(chunked->encode("Hello World!"), "c\r\nHello World!\r\n");
    EXPECT_EQ(chunked->encode(""), "0\r\n\r\n");
    
    // - with chunk extensions -
    std::vector<chunk_ext_token> extension_list = {
        chunk_ext_token("foo", "bar"),
        chunk_ext_token("baz", "\"qux\"")
    };
    chunked = transfer_encoding::chunked(
    [&](std::vector<chunk_ext_token>** extensions)->void {
        *extensions = &extension_list;
    });
    EXPECT_EQ(chunked->encode("Hello World!"),
        "c;foo=bar;baz=\"qux\"\r\nHello World!\r\n");
    EXPECT_EQ(chunked->encode(""),
        "0;foo=bar;baz=\"qux\"\r\n\r\n");
    
    // - with trailers -
    std::vector<message_header> header_list = {
        message_header("foo", "bar"),
        message_header("baz", "qux")
    };
    chunked = transfer_encoding::chunked(nullptr,
    [&](std::vector<message_header>** trailers)->void {
        *trailers = &header_list;
    });
    EXPECT_EQ(chunked->encode(""), "0\r\nfoo: bar\r\nbaz: qux\r\n\r\n");
    
    // - NULL extensions returned -
    chunked = transfer_encoding::chunked(
    [&](std::vector<chunk_ext_token>** extensions)->void {
        *extensions = NULL;
    });
    EXPECT_EQ(chunked->encode("Hello World!"), "c\r\nHello World!\r\n");
    
    // - NULL trailers returned -
    chunked = transfer_encoding::chunked(nullptr,
    [&](std::vector<message_header>** trailers)->void {
        *trailers = NULL;
    });
    EXPECT_EQ(chunked->encode(""), "0\r\n\r\n");
}