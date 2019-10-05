/**
 * Created by TekuConcept on October 4, 2019
 */

#include <sstream>
#include <gtest/gtest.h>
#include "utils/environment.h"
#include "rfc/http/transfer_pipe.h"

using namespace impact;
using namespace http;

#define NO_THROW_BEGIN try {
#define NO_THROW_END   } catch (...) { FAIL(); }
#define THROW_BEGIN    try {
#define THROW_END      FAIL(); } catch (...) { }


class dummy_coding : public transfer_coding {
public:
    dummy_coding() : transfer_coding("FooBar") { }
    std::string encode(const std::string& _) { return _; };
};


TEST(test_http_transfer_pipe, default_coding)
{
    transfer_pipe pipe;
    EXPECT_EQ(pipe.codings().size(), 1);
    EXPECT_EQ(pipe.codings().back()->name(), "chunked");
}


TEST(test_http_transfer_pipe, set_codings)
{
    { // multiple chunked codings
        transfer_pipe pipe;
        pipe.set_codings({
            new chunked_coding(), // discarded
            new chunked_coding(),
            new dummy_coding()
        });
        EXPECT_EQ(pipe.codings().size(), 2);
        // chunked is always last coding
        EXPECT_EQ(pipe.codings().front()->name(), "FooBar");
        EXPECT_EQ(pipe.codings().back()->name(), "chunked");
    }

    { // default chunked coding provided
        transfer_pipe pipe;
        pipe.set_codings({ new dummy_coding() });
        EXPECT_EQ(pipe.codings().size(), 2);
        EXPECT_EQ(pipe.codings().front()->name(), "FooBar");
        EXPECT_EQ(pipe.codings().back()->name(), "chunked");
    }
}


class dummy_sink : public transfer_pipe::sink {
public:
    dummy_sink(std::iostream& stream)
    : m_stream_(stream) { }
    void on_chunk(const std::string& __chunk)
    { m_stream_ << __chunk; }
    void on_error(const std::string&) { }
    void on_eop() { }
private:
    std::iostream& m_stream_;
};


TEST(test_http_transfer_pipe, set_sync)
{
    std::stringstream ss;
    dummy_sink sink(ss);
    transfer_pipe pipe;
    pipe.set_sink(&sink);
}





TEST(test_http_transfer_pipe, send)
{
    transfer_pipe pipe;

    std::stringstream ss;
    dummy_sink sink(ss);
    pipe.set_sink(&sink);

    EXPECT_EQ(pipe.send("Hello World!"), 0xC);
    EXPECT_EQ(pipe.send(transfer_pipe::EOP), 0); // end of payload
    EXPECT_EQ(pipe.send("Good Bye!"), 0);        // sink detatched

    EXPECT_EQ(ss.str(), "C\r\nHello World!\r\n0\r\n\r\n");
}
