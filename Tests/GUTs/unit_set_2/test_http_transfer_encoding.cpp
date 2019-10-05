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


class dummy_transfer_encoding : public transfer_encoding {
public:
    status on_data_requested(std::string* buffer)
    { (void)buffer; return status::DONE; }
};


class dummy_coding : public transfer_coding {
public:
    dummy_coding() : transfer_coding("FooBar") { }
    std::string encode(const std::string& _) { return _; };
};


TEST(test_http_transfer_encoding, default_coding)
{
    dummy_transfer_encoding encoding;
    EXPECT_EQ(encoding.codings().size(), 1);
    EXPECT_EQ(encoding.codings().back()->name(), "chunked");
}


TEST(test_http_transfer_encoding, set_codings)
{
    { // multiple chunked codings
        dummy_transfer_encoding encoding;
        encoding.set_codings({
            new chunked_coding(), // discarded
            new chunked_coding(),
            new dummy_coding()
        });
        EXPECT_EQ(encoding.codings().size(), 2);
        // chunked is always last coding
        EXPECT_EQ(encoding.codings().front()->name(), "FooBar");
        EXPECT_EQ(encoding.codings().back()->name(), "chunked");
    }

    { // default chunked coding provided
        dummy_transfer_encoding encoding;
        encoding.set_codings({ new dummy_coding() });
        EXPECT_EQ(encoding.codings().size(), 2);
        EXPECT_EQ(encoding.codings().front()->name(), "FooBar");
        EXPECT_EQ(encoding.codings().back()->name(), "chunked");
    }
}
