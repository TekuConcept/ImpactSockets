/**
 * Created by TekuConcept on December 15, 2018
 */

#include <string>
#include <sstream>
#include <vector>

#include <gtest/gtest.h>
#include "utils/environment.h"
#include "rfc/http/messenger.h"
#include "rfc/http/TX/response_message.h"

using namespace impact;
using namespace http;

#define NO_THROW(code) try { {code} } catch (...) { FAIL(); }
#define THROW(code)    try { {code} FAIL(); } catch (...) { }

TEST(test_http_response_message, create)
{
    NO_THROW( // manually defined code and phrase
        response_message response = response_message(200, "OK");
        EXPECT_EQ(response.status_code(), 200);
        EXPECT_EQ(response.reason_phrase(), "OK");
    )
    
    NO_THROW( // pre-defined code and phrase
        response_message response = response_message(status_code::OK);
        EXPECT_EQ(response.status_code(), 200);
        EXPECT_EQ(response.reason_phrase(), "OK");
    )
}


TEST(test_http_response_message, start_line)
{
    NO_THROW(
        response_message response = response_message(200, "OK");
        std::stringstream stream;
        response.send(stream);
        EXPECT_EQ(stream.str(), "HTTP/1.1 200 OK\r\n\r\n");
    )
}


TEST(test_http_response_message, type)
{
    NO_THROW(
        response_message response = response_message(200, "OK");
        EXPECT_EQ(response.type(), message_type::RESPONSE);
    )
}


TEST(test_http_response_message, valid_status_code)
{
    NO_THROW(response_message response = response_message(200, "OK");)
    // status codes must be no more than three digits
    THROW(response_message response = response_message(1000, "OK");)
}


TEST(test_http_response_message, valid_reason_phrase)
{
    NO_THROW(response_message response = response_message(200, "OK");)
    // reason phrase must contain characters in the set
    // { HTAB, SP, VCHAR, OBS_TEXT }
    THROW(
        response_message response =
            response_message(200, std::string("\x00\x7F", 2));
    )
}


TEST(test_http_response_message, send_response)
{
    // auto response = http::response_message::create(200, "OK");
    // response->send(std::cout);
}
