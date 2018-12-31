/**
 * Created by TekuConcept on December 30, 2018
 */

#include <gtest/gtest.h>
#include "utils/environment.h"
#include "rfc/http/TX/message_traits.h"

using namespace impact;
using namespace http;

#define NO_THROW(code) try { {code} } catch (...) { FAIL(); }
#define THROW(code)    try { {code} FAIL(); } catch (...) { }


TEST(test_http_method_token, method_token)
{
    //  method = token
    //  token  = 1*tchar
    //  tchar  = "!" / "#" / "$" / "%" / "&" / "'" / "*"
    //            / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"
    //            / DIGIT / ALPHA
    
    // [- GOOD -]
    
    NO_THROW(method_token token = method_token("GET");)
    
    // [- BAD -]
    
    // no method
    THROW(method_token token = method_token("");)
    // invalid method character
    THROW(method_token token = method_token(" ");)
    
    // [- INFO -]
    NO_THROW(
        method_token token = method_token("GET");
        EXPECT_EQ(token.name(), "GET");
    )
    NO_THROW( // skip method name validation check
        method_token token = method_token(method::CONNECT);
        EXPECT_EQ(token.name(), "CONNECT");
    )
}


TEST(test_http_target_token, target_token)
{
    // request-target = origin-form
    //                 / absolute-form
    //                 / authority-form
    //                 / asterisk-form
    // origin-form    = absolute-path [ "?" query ]
    // absolute-form  = absolute-URI
    // authority-form = authority
    // asterisk-form  = "*"
    
    // Examples:
    // origin-form:    /where?q=now
    // absolute-form:  http://www.example.org/pub/WWW/TheProject.html
    // authority-form: www.example.com:80
    
    NO_THROW(
        target_token token = target_token("*");
        EXPECT_EQ(token.type(), target_token::path_type::ASTERISK);
        EXPECT_EQ(token.name(), "*");
    )
    NO_THROW(
        target_token token = target_token("/");
        EXPECT_EQ(token.type(), target_token::path_type::ORIGIN);
        EXPECT_EQ(token.name(), "/");
    )
    NO_THROW(
        target_token token = target_token("/where?q=now");
        EXPECT_EQ(token.type(), target_token::path_type::ORIGIN);
        EXPECT_EQ(token.name(), "/where?q=now");
    )
    NO_THROW(
        target_token token = target_token(
            "http://www.example.org/pub/WWW/TheProject.html");
        EXPECT_EQ(token.type(), target_token::path_type::ABSOLUTE);
        EXPECT_EQ(token.name(),
            "http://www.example.org/pub/WWW/TheProject.html");
    )
    NO_THROW(
        target_token token = target_token("www.example.com:80");
        EXPECT_EQ(token.type(), target_token::path_type::AUTHORITY);
        EXPECT_EQ(token.name(), "www.example.com:80");
    )
    THROW(target_token token = target_token("//bad/origin");)
    THROW(target_token token = target_token("");)
    THROW(target_token token = target_token(" ");)
}


TEST(test_http_request_message, valid_method)
{
    // NOTE: See also method_token
    NO_THROW(request_traits request = request_traits("GET", "/");)
    THROW(request_traits request = request_traits("", "/");)
}


TEST(test_http_request_message, valid_target)
{
    // NOTE: See also target_token
    NO_THROW(request_traits request = request_traits("_", "/");)
    THROW(request_traits request = request_traits("_", "//bad/origin");)
}


TEST(test_http_request_message, start_line)
{
    NO_THROW(
        request_traits request = request_traits("GET", "/");
        EXPECT_EQ(request.start_line(), "GET / HTTP/1.1");
    )
}


TEST(test_http_request_message, type)
{
    NO_THROW(
        request_traits request = request_traits("GET", "/");
        EXPECT_EQ(request.type(), message_type::REQUEST);
    )
}


TEST(test_http_response_message, create)
{
    NO_THROW( // manually defined code and phrase
        response_traits response = response_traits(200, "OK");
        EXPECT_EQ(response.status_code(), 200);
        EXPECT_EQ(response.reason_phrase(), "OK");
    )
    
    NO_THROW( // pre-defined code and phrase
        response_traits response = response_traits(status_code::OK);
        EXPECT_EQ(response.status_code(), 200);
        EXPECT_EQ(response.reason_phrase(), "OK");
    )
}


TEST(test_http_response_message, start_line)
{
    NO_THROW(
        response_traits response = response_traits(200, "OK");
        EXPECT_EQ(response.start_line(), "HTTP/1.1 200 OK");
    )
}


TEST(test_http_response_message, type)
{
    NO_THROW(
        response_traits response = response_traits(200, "OK");
        EXPECT_EQ(response.type(), message_type::RESPONSE);
    )
}


TEST(test_http_response_message, valid_status_code)
{
    NO_THROW(response_traits response = response_traits(200, "OK");)
    // status codes must be no more than three digits
    THROW(response_traits response = response_traits(1000, "OK");)
}


TEST(test_http_response_message, valid_reason_phrase)
{
    NO_THROW(response_traits response = response_traits(200, "OK");)
    // reason phrase must contain characters in the set
    // { HTAB, SP, VCHAR, OBS_TEXT }
    THROW(
        response_traits response =
            response_traits(200, std::string("\x00\x7F", 2));
    )
}
