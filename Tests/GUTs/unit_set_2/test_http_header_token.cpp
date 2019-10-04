/**
 * Created by TekuConcept on December 29, 2018
 */

#include <gtest/gtest.h>
#include "utils/environment.h"
#include "rfc/http/header.h"

using namespace impact;
using namespace http;

#define NO_THROW(code) try { {code} } catch (...) { FAIL(); }
#define THROW(code)    try { {code} FAIL(); } catch (...) { }

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

    NO_THROW( // case-insensitive
        header_t token("name", "value");
        EXPECT_EQ(token.name(), "Name");
        EXPECT_EQ(token.value(), "value");
    )

    NO_THROW( // field value whitespace trimming
        header_t token("name", "\tset value ");
        EXPECT_EQ(token.value(), "set value");
    )

    NO_THROW( // skip field name validation check
        header_t token(field_name::CONNECTION, "value");
        EXPECT_EQ(token.name(), "Connection");
        EXPECT_EQ(token.value(), "value");
    )

    THROW(header_t token("/bad", "value");) // TCHAR only

    THROW(header_t token("name", "\x7F");)  // VCHAR only
    
    // raw header line
    NO_THROW(header_t token("name: value\r\n");)
    THROW(header_t token("name: value");)

    // optional white space
    NO_THROW(
        header_t token("name:   cats, dogs   \r\n");
        EXPECT_EQ(token.name(), "name");
        EXPECT_EQ(token.value(), "cats, dogs");
    )

    // obsolete line folding
    NO_THROW(
        header_t token("name: value1,\r\n\tvalue2\r\n");
        EXPECT_EQ(token.name(), "name");
        EXPECT_EQ(token.value(), "value1, value2");
    )
    NO_THROW(
        header_t token("name: value1,\r\n value2\r\n");
        EXPECT_EQ(token.name(), "name");
        EXPECT_EQ(token.value(), "value1, value2");
    )
    // obsolete line folding must contain SP or HTAB on next line
    THROW(header_t token("name: bad,\r\nvalue\r\n");)

    // no spaces between name and ':'
    THROW(header_t token("name : bad\r\n");)

    NO_THROW(
        header_t token("name", "value");
        EXPECT_EQ(token.to_string(), "name: value\r\n");
    )
}
