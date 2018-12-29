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

#define NO_THROW(code) try { {code} } catch (...) { FAIL(); }
#define THROW(code)    try { {code} FAIL(); } catch (...) { }

TEST(test_http_response_message, transfer_encoding)
{
    // empty encoding set
    // null data callback
    // custom encoding
    // duplicate chunked encodings
    // duplicate other encodings
    // custom encoding called "chunked" (hint: check name in ctor)
}


TEST(test_http_response_message, fixed_message)
{
    // create fixed-data message
}


TEST(test_http_response_message, headers)
{
    // generic header format
    // add headers to message
    // remove headers from message
    // access message headers
    // handle duplicate headers?
}


TEST(test_http_response_message, send_response)
{
    // auto response = http::response_message::create(200, "OK");
    // response->send(std::cout);
}
