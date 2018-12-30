/**
 * Created by TekuConcept on December 29, 2018
 */

#include <gtest/gtest.h>
#include "utils/environment.h"
#include "rfc/http/TX/message.h"

using namespace impact;
using namespace http;

#define NO_THROW(code) try { {code} } catch (...) { FAIL(); }
#define THROW(code)    try { {code} FAIL(); } catch (...) { }

TEST(test_http_transfer_encoding_token, transfer_encoding_token)
{
    {
        transfer_encoding_token token({});
        ASSERT_EQ(token.encodings().size(), 1UL);
        EXPECT_EQ(token.encodings()[0]->name(), "chunked");
        auto header = token.header();
        EXPECT_EQ(header.field_name(), "Transfer-Encoding");
        EXPECT_EQ(header.field_value(), "chunked");
        EXPECT_EQ(token.callback, nullptr);
    }
    
    {
        transfer_encoding_token token({
            transfer_encoding::chunked(),
            transfer_encoding::chunked()
        });
        ASSERT_EQ(token.encodings().size(), 1UL);
        EXPECT_EQ(token.encodings()[0]->name(), "chunked");
    }

    transfer_encoding_ptr custom = std::make_shared<transfer_encoding>(
        "custom",
        [](const std::string& data) -> std::string { return data; }
    );
    
    {
        transfer_encoding_token token({ custom, custom });
        ASSERT_EQ(token.encodings().size(), 3UL);
        EXPECT_EQ(token.encodings()[0]->name(), "custom");
        EXPECT_EQ(token.encodings()[1]->name(), "custom");
        EXPECT_EQ(token.encodings()[2]->name(), "chunked");
        auto header = token.header();
        EXPECT_EQ(header.field_name(), "Transfer-Encoding");
        EXPECT_EQ(header.field_value(), "custom, custom, chunked");
    }
}


TEST(test_http_request_message, send)
{
    // null data callback
}


TEST(test_http_request_message, fixed_message)
{
    // create fixed-data message
}


TEST(test_http_request_message, headers)
{
    // generic header format
    // add headers to message
    // remove headers from message
    // access message headers
    // handle duplicate headers?
}
