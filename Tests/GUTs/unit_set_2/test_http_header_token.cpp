/**
 * Created by TekuConcept on December 29, 2018
 */

#include <sstream>
#include <gtest/gtest.h>
#include "utils/environment.h"
#include "rfc/http/header.h"

using namespace impact;
using namespace http;

#define NO_THROW_BEGIN try {
#define NO_THROW_END   } catch (...) { FAIL(); }
#define THROW_BEGIN   try {
#define THROW_END     FAIL(); } catch (...) { }

TEST(test_http_header, header_t)
{
    /* Header Field ABNF
     header-field   = field-name ":" OWS field-value OWS

     field-name     = token
     field-value    = *( field-content / obs-fold )
     field-content  = field-vchar [ 1*( SP / HTAB ) field-vchar ]
     field-vchar    = VCHAR / obs-text

     obs-fold       = CRLF 1*( SP / HTAB )
                    ; obsolete line folding
    */

    NO_THROW_BEGIN // case-insensitive
        header_t token("name", "value");
        EXPECT_EQ(token.name(), "Name");
        EXPECT_EQ(token.value(), "value");
    NO_THROW_END

    NO_THROW_BEGIN // field value whitespace trimming
        header_t token("name", "\tset value ");
        EXPECT_EQ(token.value(), "set value");
    NO_THROW_END

    NO_THROW_BEGIN // skip field name validation check
        header_t token(field_name::CONNECTION, "value");
        EXPECT_EQ(token.name(), "Connection");
        EXPECT_EQ(token.value(), "value");
    NO_THROW_END

    NO_THROW_BEGIN // raw header line
        header_t token("name: value\r\n");
    NO_THROW_END

    NO_THROW_BEGIN // optional white space
        header_t token("name:   cats, dogs   \r\n");
        EXPECT_EQ(token.name(), "name");
        EXPECT_EQ(token.value(), "cats, dogs");
    NO_THROW_END

    NO_THROW_BEGIN // obsolete line folding
        header_t token("name: value1,\r\n\tvalue2\r\n");
        EXPECT_EQ(token.name(), "name");
        EXPECT_EQ(token.value(), "value1, value2");
    NO_THROW_END

    NO_THROW_BEGIN // obsolete line folding
        header_t token("name: value1,\r\n value2\r\n");
        EXPECT_EQ(token.name(), "name");
        EXPECT_EQ(token.value(), "value1, value2");
    NO_THROW_END

    NO_THROW_BEGIN
        header_t token("name", "value");
        EXPECT_EQ(token.to_string(), "name: value\r\n");
    NO_THROW_END

    THROW_BEGIN
        header_t token("/bad", "value"); // TCHAR only
    THROW_END

    THROW_BEGIN
        header_t token("name", "\x7F");  // VCHAR only
    THROW_END

    THROW_BEGIN
        header_t token("name: value"); // missing CRLF
    THROW_END

    // obsolete line folding must contain SP or HTAB on next line
    THROW_BEGIN
        header_t token("name: bad,\r\nvalue\r\n");
    THROW_END

    // no spaces between name and ':'
    THROW_BEGIN
        header_t token("name : bad\r\n");
    THROW_END
}


TEST(test_http_header, append_value)
{
    NO_THROW_BEGIN
        header_t token("name", "");
        token.append_value("john");
        EXPECT_EQ(token.value(), "john");
    NO_THROW_END

    NO_THROW_BEGIN
        header_t token("name", "john");
        token.append_value("marry");
        EXPECT_EQ(token.value(), "john, marry");
    NO_THROW_END

    {
        header_t token1("name", "john");
        header_t token2("name", "marry");
        token1.append_value_from(token2);
        EXPECT_EQ(token1.value(), "john, marry");
    }

    THROW_BEGIN
        header_t token("name", "john");
        token.append_value("\x7F");
    THROW_END
}


TEST(test_http_header_list, headers)
{
    NO_THROW_BEGIN
        /*
            A sender MUST NOT generate multiple header fields with the same field
            name in a message unless either the entire field value for that
            header field is defined as a comma-separated list [i.e., #(values)]
            or the header field is a well-known exception.

            A recipient MAY combine multiple header fields with the same field
            name into one "field-name: field-value" pair, without changing the
            semantics of the message, by appending each subsequent field value to
            the combined field value in order, separated by a comma.  The order
            in which header fields with the same field name are received is
            therefore significant to the interpretation of the combined field
            value; a proxy MUST NOT change the order of these field values when
            forwarding a message.
        */
        std::ostringstream os;
        header_list headers({
            header_t(field_name::AGE, "45"),
            header_t(field_name::AGE, "50")
        });
        for (const auto& header : headers)
            os << header;
        EXPECT_EQ(os.str(), "Age: 45, 50\r\n");
    NO_THROW_END

    NO_THROW_BEGIN
        /*
            In practice, the "Set-Cookie" header field ([RFC6265]) often
            appears multiple times in a response message and does not use the
            list syntax, violating the above requirements on multiple header
            fields with the same name.  Since it cannot be combined into a
            single field-value, recipients ought to handle "Set-Cookie" as a
            special case while processing header fields.
         */
        std::ostringstream os;
        header_list headers({
            header_t(field_name::SET_COOKIE, "foo=45"),
            header_t(field_name::SET_COOKIE, "bar=50")
        });
        for (const auto& header : headers)
            os << header;
        EXPECT_EQ(os.str(),
            "Set-Cookie: foo=45\r\n"
            "Set-Cookie: bar=50\r\n");
    NO_THROW_END

    NO_THROW_BEGIN // verify body size collision with push_back / pop_back
        header_list headers({
            header_t(field_name::AGE, "30"),
            header_t(field_name::SET_COOKIE, "bar=50")
        });
        headers.push_back(header_t(field_name::CONTENT_LENGTH, "20"));
        headers.pop_back();
        headers.push_back(header_t(field_name::CONTENT_LENGTH, "25"));
    NO_THROW_END

    NO_THROW_BEGIN // verify body size collision with insert / erase
        header_list headers;
        headers.insert(headers.begin(), {
            header_t(field_name::CONTENT_LENGTH, "20"),
            header_t(field_name::AGE, "30")
        });
        headers.erase(headers.begin());
        headers.insert(
            headers.end(),
            header_t(field_name::CONTENT_LENGTH, "25"));
    NO_THROW_END

    THROW_BEGIN
        /* Cannot have both transfer-encoding and content-length */
        std::ostringstream os;
        header_list headers({
            header_t(field_name::TRANSFER_ENCODING, "chunked"),
            header_t(field_name::CONTENT_LENGTH, "50")
        });
        for (const auto& header : headers)
            os << header;
        EXPECT_EQ(os.str(), "");
    THROW_END

    THROW_BEGIN // body size collision using push_back
        header_list headers({
            header_t(field_name::AGE, "30"),
            header_t(field_name::CONTENT_LENGTH, "50")
        });
        headers.push_back(header_t(field_name::CONTENT_LENGTH, "20"));
    THROW_END

    THROW_BEGIN // verify body size collision with insert
        header_list headers;
        headers.insert(headers.begin(), {
            header_t(field_name::CONTENT_LENGTH, "20"),
            header_t(field_name::CONTENT_LENGTH, "25")
        });
    THROW_END
}
