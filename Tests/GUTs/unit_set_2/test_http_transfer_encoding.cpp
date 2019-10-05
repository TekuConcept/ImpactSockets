/**
 * Created by TekuConcept on October 4, 2019
 */

#include <gtest/gtest.h>
#include "utils/environment.h"
#include "rfc/http/transfer_encoding.h"

using namespace impact;
using namespace http;

#define NO_THROW_BEGIN try {
#define NO_THROW_END   } catch (...) { FAIL(); }
#define THROW_BEGIN   try {
#define THROW_END     FAIL(); } catch (...) { }

TEST(test_http_transfer_encoding, transfer_encoding)
{

}
