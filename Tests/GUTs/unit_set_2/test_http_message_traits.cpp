/**
 * Created by TekuConcept on December 30, 2018
 */

#include <sstream>
#include <string>
#include <gtest/gtest.h>
#include "utils/environment.h"
#include "rfc/http/message_traits.h"

using namespace impact;
using namespace http;

#define NO_THROW(code) try { {code} } catch (...) { FAIL(); }
#define THROW(code)    try { {code} FAIL(); } catch (...) { }


TEST(test_http_method_token, method_t)
{
    /* Method ABNF
      method = token
      token  = 1*tchar
      tchar  = "!" / "#" / "$" / "%" / "&" / "'" / "*"
                / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"
                / DIGIT / ALPHA
    */

    // [- GOOD -]

    NO_THROW(message_traits::method_t token("GET");)

    // [- BAD -]

    // no method
    THROW(message_traits::method_t token("");)
    // invalid method character
    THROW(message_traits::method_t token(" ");)

    // [- INFO -]
    NO_THROW(
        message_traits::method_t token("GET");
        EXPECT_EQ(token.name(), "GET");
    )
    NO_THROW( // skip method name validation check
        message_traits::method_t token(method::CONNECT);
        EXPECT_EQ(token.name(), "CONNECT");
    )
}


TEST(test_http_target_token, target_t)
{
    /* Target ABNF
      request-target = origin-form
                     / absolute-form
                     / authority-form
                     / asterisk-form
      origin-form    = absolute-path [ "?" query ]
      absolute-form  = absolute-URI
      authority-form = authority
      asterisk-form  = "*"
    
      Examples:
      origin-form:    /where?q=now
      absolute-form:  http://www.example.org/pub/WWW/TheProject.html
      authority-form: www.example.com:80
    */
    
    NO_THROW(
        message_traits::target_t token("*");
        EXPECT_EQ(token.type(),
            message_traits::target_t::path_type::ASTERISK);
        EXPECT_EQ(token.name(), "*");
    )
    NO_THROW(
        message_traits::target_t token("/");
        EXPECT_EQ(token.type(),
            message_traits::target_t::path_type::ORIGIN);
        EXPECT_EQ(token.name(), "/");
    )
    NO_THROW(
        message_traits::target_t token("/where?q=now");
        EXPECT_EQ(token.type(),
            message_traits::target_t::path_type::ORIGIN);
        EXPECT_EQ(token.name(), "/where?q=now");
    )
    NO_THROW(
        message_traits::target_t token(
            "http://www.example.org/pub/WWW/TheProject.html");
        EXPECT_EQ(token.type(),
            message_traits::target_t::path_type::ABSOLUTE);
        EXPECT_EQ(token.name(),
            "http://www.example.org/pub/WWW/TheProject.html");
    )
    NO_THROW(
        message_traits::target_t token("www.example.com:80");
        EXPECT_EQ(token.type(),
            message_traits::target_t::path_type::AUTHORITY);
        EXPECT_EQ(token.name(), "www.example.com:80");
    )
    THROW(message_traits::target_t token("//bad/origin");)
    THROW(message_traits::target_t token("");)
    THROW(message_traits::target_t token(" ");)
}

TEST(test_http_message_traits, streams)
{
    request_traits request("GET", "/");
    response_traits response(200, "OK");

    std::ostringstream os;
    os << request << response;
    EXPECT_EQ(os.str(), "GET / HTTP/1.1\r\nHTTP/1.1 200 OK\r\n");
}


TEST(test_http_request_traits, valid_method)
{
    // NOTE: See also method_t
    NO_THROW(request_traits request("GET", "/");)
    THROW(request_traits request("", "/");)
}


TEST(test_http_request_traits, valid_target)
{
    // NOTE: See also target_t
    NO_THROW(request_traits request("_", "/");)
    THROW(request_traits request("_", "//bad/origin");)
}


TEST(test_http_request_traits, to_string)
{
    NO_THROW(
        request_traits request("GET", "/");
        EXPECT_EQ(request.to_string(), "GET / HTTP/1.1\r\n");
    )
    
    NO_THROW(
        auto ptr = message_traits::create("GET / HTTP/1.1\r\n");
        ASSERT_EQ(ptr->type(), message_traits::message_type::REQUEST);
        request_traits& request = *(request_traits*)ptr.get();
        EXPECT_EQ(request.method(), "GET");
        EXPECT_EQ(request.target(), "/");
        EXPECT_EQ(request.http_major(), 1);
        EXPECT_EQ(request.http_minor(), 1);
    )
    
    THROW(auto ptr = message_traits::create("GET / HTTP");)
}


TEST(test_http_request_traits, type)
{
    NO_THROW(
        request_traits request("GET", "/");
        EXPECT_EQ(request.type(), message_traits::message_type::REQUEST);
    )
}


TEST(test_http_request_traits, permit)
{
    request_traits traits("HEAD", "/");
    EXPECT_TRUE(traits.permit_user_length_header());
    EXPECT_TRUE(traits.permit_length_header());
    EXPECT_FALSE(traits.permit_body());
    traits = request_traits("GET", "/");
    EXPECT_FALSE(traits.permit_user_length_header());
    EXPECT_TRUE(traits.permit_length_header());
    EXPECT_TRUE(traits.permit_body());
    traits = request_traits("TRACE", "/");
    EXPECT_TRUE(traits.permit_length_header());
    EXPECT_FALSE(traits.permit_body());
}


TEST(test_http_response_traits, create)
{
    NO_THROW( // manually defined code and phrase
        response_traits response(200, "OK");
        EXPECT_EQ(response.code(), 200);
        EXPECT_EQ(response.reason_phrase(), "OK");
    )
    
    NO_THROW( // pre-defined code and phrase
        response_traits response(status_code::OK);
        EXPECT_EQ(response.code(), 200);
        EXPECT_EQ(response.reason_phrase(), "OK");
    )
}


TEST(test_http_response_traits, to_string)
{
    NO_THROW(
        response_traits response(200, "OK");
        EXPECT_EQ(response.to_string(), "HTTP/1.1 200 OK\r\n");
    )
    
    NO_THROW(
        auto ptr = message_traits::create("HTTP/1.1 200 OK\r\n");
        ASSERT_EQ(ptr->type(), message_traits::message_type::RESPONSE);
        response_traits& response = *(response_traits*)ptr.get();
        EXPECT_EQ(response.code(), 200);
        EXPECT_EQ(response.reason_phrase(), "OK");
        EXPECT_EQ(response.http_major(), 1);
        EXPECT_EQ(response.http_minor(), 1);
    )
    
    THROW(auto ptr = message_traits::create("HTTP 200");)
}


TEST(test_http_response_traits, type)
{
    NO_THROW(
        response_traits response = response_traits(200, "OK");
        EXPECT_EQ(response.type(), message_traits::message_type::RESPONSE);
    )
}


TEST(test_http_response_traits, valid_status_code)
{
    NO_THROW(response_traits response = response_traits(200, "OK");)
    // status codes must be no more than three digits
    THROW(response_traits response = response_traits(1000, "OK");)
}


TEST(test_http_response_traits, valid_reason_phrase)
{
    NO_THROW(response_traits response = response_traits(200, "OK");)
    // reason phrase must contain characters in the set
    // { HTAB, SP, VCHAR, OBS_TEXT }
    THROW(
        response_traits response =
            response_traits(200, std::string("\x00\x7F", 2));
    )
}


TEST(test_http_response_traits, permit)
{
    response_traits traits(304, "Not Modified");
    EXPECT_TRUE(traits.permit_user_length_header());
    EXPECT_FALSE(traits.permit_body());
    traits = response_traits(200, "OK");
    EXPECT_FALSE(traits.permit_user_length_header());
    EXPECT_TRUE(traits.permit_length_header());
    EXPECT_TRUE(traits.permit_body());
    traits = response_traits(204, "No Content");
    EXPECT_FALSE(traits.permit_length_header());
    traits = response_traits(100, "Continue");
    EXPECT_FALSE(traits.permit_length_header());
}
