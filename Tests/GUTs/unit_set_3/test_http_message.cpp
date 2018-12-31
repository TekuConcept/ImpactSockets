/**
 * Created by TekuConcept on December 31, 2018
 */

#include <gtest/gtest.h>
#include "rfc/http/RX/messaging.h"

using namespace impact;
using namespace http;

#define NO_THROW(code) try { {code} } catch (...) { FAIL(); }
#define THROW(code)    try { {code} FAIL(); } catch (...) { }


TEST(test, generic)
{
    // HTTP/1.1 200 OK\r\n
    // Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n
    // Server: Apache\r\n
    // Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n
    // ETag: "34aa387-d-1568eb00"\r\n
    // Accept-Ranges: bytes\r\n
    // Content-Length: 51\r\n
    // Vary: Accept-Encoding\r\n
    // Content-Type: text/plain\r\n
    // \r\n
    // Hello World! My payload includes a trailing CRLF.\r\n
}
