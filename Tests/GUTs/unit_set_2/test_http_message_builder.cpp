/**
 * Created by TekuConcept on October 5, 2019
 */

#include <sstream>
#include <iostream>
#include <gtest/gtest.h>
#include "utils/environment.h"
#include "rfc/http/message_builder.h"

using namespace impact;
using namespace http;

#define NO_THROW_BEGIN try {
#define NO_THROW_END   } catch (...) { FAIL(); }
#define THROW_BEGIN   try {
#define THROW_END     FAIL(); } catch (...) { }


TEST(test_http_message_builder, message_builder)
{
    message_builder builder;
    std::string buffer =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Field: obsolete,\r\n"
        "    line-folding\r\n"
        "Check: ok\r\n"
        "\r\n";
    std::stringstream ss;

    std::cout << "trial 1\n";
    builder.write(buffer);
    EXPECT_EQ(builder.buffer().size(), 0);

    std::cout << "trial 2\n";
    builder.clear();
    builder.write(buffer.begin(), buffer.end());
    EXPECT_EQ(builder.buffer().size(), 0);

    std::cout << "trial 3\n";
    builder.clear();
    ss << buffer;
    ss >> builder;
    EXPECT_EQ(builder.buffer().size(), 0);
}
