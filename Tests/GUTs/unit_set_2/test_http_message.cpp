/**
 * Created by TekuConcept on October 3, 2019
 */

#include <gtest/gtest.h>
#include "utils/environment.h"
#include "rfc/http/message.h"

using namespace impact;
using namespace http;

#define NO_THROW(code) try { {code} } catch (...) { FAIL(); }
#define THROW(code)    try { {code} FAIL(); } catch (...) { }

TEST(test_http_message, message_t)
{
    NO_THROW(
        std::ostringstream os;
        message_t message("GET", "/test");
        os << message;
        EXPECT_EQ(os.str(), "GET /test HTTP/1.1\r\n\r\n");
    )

    NO_THROW(
        std::ostringstream os;
        message_t message(method::GET, "/test");
        os << message;
        EXPECT_EQ(os.str(), "GET /test HTTP/1.1\r\n\r\n");
    )

    NO_THROW(
        std::ostringstream os;
        message_t message(
            "GET"
            , "/test"
            , {
                header_t(field_name::HOST, "localhost"),
                header_t(field_name::ACCEPT_ENCODING, "utf8")
            }
        );
        os << message;
        EXPECT_EQ(os.str(),
            "GET /test HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Accept-Encoding: utf8\r\n"
            "\r\n");
    )

    NO_THROW(
        std::ostringstream os;
        message_t message(200, "OK");
        os << message;
        EXPECT_EQ(os.str(), "HTTP/1.1 200 OK\r\n\r\n");
    )

    NO_THROW(
        std::ostringstream os;
        message_t message(status_code::OK);
        os << message;
        EXPECT_EQ(os.str(), "HTTP/1.1 200 OK\r\n\r\n");
    )
}
