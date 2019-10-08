/**
 * Created by TekuConcept on October 4, 2019
 */

#include <iostream>
#include <regex>

#include <sstream>
#include <gtest/gtest.h>
#include "utils/environment.h"
#include "rfc/http/transfer_coding.h"

using namespace impact;
using namespace http;

#define NO_THROW_BEGIN try {
#define NO_THROW_END   } catch (...) { FAIL(); }
#define THROW_BEGIN   try {
#define THROW_END     FAIL(); } catch (...) { }


class good_transfer_coding : public transfer_coding {
public:
    good_transfer_coding()
    : transfer_coding("FooBar") { }
    ~good_transfer_coding() = default;
    std::string encode(const std::string& _) { return _; };
};


class bad_transfer_coding : public transfer_coding {
public:
    bad_transfer_coding()
    : transfer_coding("\x7F") { } // TCHAR only
    ~bad_transfer_coding() = default;
    std::string encode(const std::string& _) { return _; };
};


class reserved_transfer_coding : public transfer_coding {
public:
    reserved_transfer_coding()
    : transfer_coding("chunked") { } // chunked name reserved
    ~reserved_transfer_coding() = default;
    std::string encode(const std::string& _) { return _; };
};


TEST(test_http_transfer_coding, basic_coding)
{
    NO_THROW_BEGIN
        good_transfer_coding good;
    NO_THROW_END

    THROW_BEGIN
        bad_transfer_coding bad;
    THROW_END

    THROW_BEGIN
        reserved_transfer_coding bad;
    THROW_END

    NO_THROW_BEGIN
        auto passthrough = transfer_coding::create_passthrough("test");
        EXPECT_EQ(passthrough->encode("Hello World!"), "Hello World!");
    NO_THROW_END

    THROW_BEGIN
        // any reserved name other than chunked is allowed
        auto passthrough = transfer_coding::create_passthrough("chunked");
    THROW_END
}


TEST(test_http_chunked_coding, chunked_extension)
{
    NO_THROW_BEGIN
        chunked_coding::extension_t extension("foo");
        EXPECT_EQ(extension.to_string(), ";foo");
    NO_THROW_END

    NO_THROW_BEGIN
        chunked_coding::extension_t extension("foo", "bar");
        EXPECT_EQ(extension.to_string(), ";foo=bar");
    NO_THROW_END

    NO_THROW_BEGIN
        chunked_coding::extension_t extension("foo", "\"bar\"");
        EXPECT_EQ(extension.to_string(), ";foo=\"bar\"");
    NO_THROW_END

    THROW_BEGIN
        chunked_coding::extension_t extension("foo", "\"ba\"r");
    THROW_END

    NO_THROW_BEGIN
        chunked_coding::extension_t extension("foo", "\"bar\"");
        EXPECT_EQ(extension.to_string(), ";foo=\"bar\"");
        extension.value("baz");
        EXPECT_EQ(extension.to_string(), ";foo=baz");
    NO_THROW_END

    THROW_BEGIN
        chunked_coding::extension_t extension("foo", "bar");
        extension.value("ba\"z");
    THROW_END

    NO_THROW_BEGIN
        chunked_coding::extension_t extension =
            chunked_coding::extension_t::parse(";foo=bar");
        EXPECT_EQ(extension.name(), "foo");
        EXPECT_EQ(extension.value(), "bar");
    NO_THROW_END

    THROW_BEGIN
        chunked_coding::extension_t extension =
            chunked_coding::extension_t::parse("foo=b\"ar");
    THROW_END
}


class dummy_chunked_observer : public chunked_observer {
public:
    ~dummy_chunked_observer() = default;

    void
    on_next_chunk(
        std::vector<chunked_coding::extension_t>& __extensions,
        const std::string&                        __buffer)
    {
        __extensions.push_back(chunked_coding::extension_t("foo"));
        __extensions.push_back(chunked_coding::extension_t("bar", "baz"));
        (void)__buffer;
    }

    void
    on_last_chunk(
        std::vector<chunked_coding::extension_t>& __extensions,
        header_list&                              __trailers)
    {
        __extensions.push_back(chunked_coding::extension_t("daisy"));
        __trailers.insert(__trailers.begin(), {
            // content-length is a forbidden trailer;
            // it will be discarded
            header_t(field_name::CONTENT_LENGTH, "42"),
            header_t("Upload-Status", "OK")
        });
    }
};


TEST(test_http_chunked_coding, chunked_coding)
{
    NO_THROW_BEGIN
        chunked_coding coding;
    NO_THROW_END

    {
        chunked_coding coding;
        dummy_chunked_observer observer;

        // [-- vanilla --]

        // basic message
        auto chunk = coding.encode("Hello World!");
        EXPECT_EQ(chunk, "C\r\nHello World!\r\n");

        // last message
        chunk = coding.encode("");
        EXPECT_EQ(chunk, "0\r\n\r\n");

        // [-- extended --]

        coding.register_observer(&observer);

        // basic message
        chunk = coding.encode("Hello World!");
        EXPECT_EQ(chunk, "C;foo;bar=baz\r\nHello World!\r\n");

        // last message
        chunk = coding.encode("");
        EXPECT_EQ(chunk,
            "0;daisy\r\n"
            "Upload-Status: OK\r\n"
            "\r\n");
    }
}


TEST(test_http_chunked_coding, chunk_header)
{
    size_t size;
    std::vector<chunked_coding::extension_t> extensions;

    // plain chunk header
    NO_THROW_BEGIN
    chunked_coding::parse_chunk_header(
        "C\r\n",
        &size, &extensions);
    EXPECT_EQ(size, 0xC);
    EXPECT_EQ(extensions.size(), 0);
    NO_THROW_END

    // last chunk header
    NO_THROW_BEGIN
    chunked_coding::parse_chunk_header(
        "0\r\n",
        &size, &extensions);
    EXPECT_EQ(size, 0x0);
    EXPECT_EQ(extensions.size(), 0);
    NO_THROW_END

    // one chunk extension
    NO_THROW_BEGIN
    chunked_coding::parse_chunk_header(
        "C;foo\r\n",
        &size, &extensions);
    EXPECT_EQ(size, 0xC);
    EXPECT_EQ(extensions.size(), 1);
    EXPECT_EQ(extensions.back().name(), "foo");
    NO_THROW_END

    // chunk extensions with values
    NO_THROW_BEGIN
    chunked_coding::parse_chunk_header(
        "C;foo=bar;baz;name=\"mary;john\"\r\n",
        &size, &extensions);
    EXPECT_EQ(size, 0xC);
    EXPECT_EQ(extensions.size(), 3);
    EXPECT_EQ(extensions[0].name(),  "foo");
    EXPECT_EQ(extensions[0].value(), "bar");
    EXPECT_EQ(extensions[1].name(),  "baz");
    EXPECT_EQ(extensions[1].value(), "");
    EXPECT_EQ(extensions[2].name(),  "name");
    EXPECT_EQ(extensions[2].value(), "\"mary;john\"");
    NO_THROW_END

    // bad extension name
    THROW_BEGIN
    chunked_coding::parse_chunk_header(
        "C;\r\n", &size, &extensions);
    THROW_END

    // bad extension value
    THROW_BEGIN
    chunked_coding::parse_chunk_header(
        "C;foo=\r\n", &size, &extensions);
    THROW_END

    // bad extension name 2
    THROW_BEGIN
    chunked_coding::parse_chunk_header(
        "C;foo=bar;\r\n", &size, &extensions);
    THROW_END

    // incomplete quote
    THROW_BEGIN
    chunked_coding::parse_chunk_header(
        "C;foo=\"test\r\n", &size, &extensions);
    THROW_END

    // incomplete quote 2
    THROW_BEGIN
    chunked_coding::parse_chunk_header(
        "C;foo=\"test;bar=baz\r\n", &size, &extensions);
    THROW_END

    // incomplete quoted pair (hanging escape '\')
    THROW_BEGIN
    chunked_coding::parse_chunk_header(
        "C;foo=\"\\\";\r\n", &size, &extensions);
    THROW_END
}
