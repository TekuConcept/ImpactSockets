/**
 * Created by TekuConcept on July 28, 2017
 */

#include <gtest/gtest.h>
#include <rfc/sha1.h>

using namespace impact;

TEST(test_sha1, digest)
{
    std::string message = "hello world";
    EXPECT_EQ(sha1::digest(message),
        "\x2a\xae\x6c\x35\xc9"
        "\x4f\xcf\xb4\x15\xdb"
        "\xe9\x5f\x40\x8b\x9c"
        "\xe9\x1e\xe8\x46\xed");

    message = "dGhlIHNhbXBsZSBub25jZQ==258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    EXPECT_EQ(sha1::digest(message),
        "\xb3\x7a\x4f\x2c\xc0"
        "\x62\x4f\x16\x90\xf6"
        "\x46\x06\xcf\x38\x59"
        "\x45\xb2\xbe\xc4\xea");
}
