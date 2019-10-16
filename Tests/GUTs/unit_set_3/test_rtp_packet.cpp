/**
 * Created by TekuConcept on October 14, 2019
 */

#include <string>
#include <gtest/gtest.h>

#include "rfc/rtp/packet.h"

using namespace impact;
using namespace rtp;

#define NO_THROW_BEGIN try {
#define NO_THROW_END   } catch (...) { FAIL(); }
#define THROW_BEGIN   try {
#define THROW_END     FAIL(); } catch (...) { }


TEST(test_rtp_packet, packet)
{
    packet_t::header header;
    EXPECT_EQ(header.version, 2);
    EXPECT_EQ(header.padding, 0);
    EXPECT_EQ(header.extension, 0);
    EXPECT_EQ(header.contributing_source_count, 0);
    EXPECT_EQ(header.marker, 0);
    EXPECT_EQ(header.payload_type, 0);
    EXPECT_EQ(header.sequence_number, 0);
    EXPECT_EQ(header.timestamp, 0);
    EXPECT_EQ(header.synchronization_source, 0);

    NO_THROW_BEGIN
        header = packet_t::header({
            0xB5, 0x96, 0xAC, 0xE1,
            0x01, 0x23, 0x45, 0x67,
            0x89, 0xAB, 0xCD, 0xEF
        });
        EXPECT_EQ(header.version, 2);
        EXPECT_EQ(header.padding, 1);
        EXPECT_EQ(header.extension, 1);
        EXPECT_EQ(header.contributing_source_count, 5);
        EXPECT_EQ(header.marker, 1);
        EXPECT_EQ(header.payload_type, 22);
        EXPECT_EQ(header.sequence_number, 0xACE1);
        EXPECT_EQ(header.timestamp, 0x01234567);
        EXPECT_EQ(header.synchronization_source, 0x89ABCDEF);
    NO_THROW_END

    NO_THROW_BEGIN
        std::string bitstream(
            "\xB5\x96\xAC\xE1"
            "\x01\x23\x45\x67"
            "\x89\xAB\xCD\xEF",
            12
        );
        packet_t::header header(bitstream.begin(), bitstream.end());
        EXPECT_EQ(header.version, 2);
        EXPECT_EQ(header.padding, 1);
        EXPECT_EQ(header.extension, 1);
        EXPECT_EQ(header.contributing_source_count, 5);
        EXPECT_EQ(header.marker, 1);
        EXPECT_EQ(header.payload_type, 22);
        EXPECT_EQ(header.sequence_number, 0xACE1);
        EXPECT_EQ(header.timestamp, 0x01234567);
        EXPECT_EQ(header.synchronization_source, 0x89ABCDEF);
    NO_THROW_END

    THROW_BEGIN
        // insufficient data
        std::string bitstream("abc");
        packet_t::header header(bitstream.begin(), bitstream.end());
    THROW_END

    THROW_BEGIN
        // unsupported version
        packet_t::header header({
            0x05, 0x96, 0xAC, 0xE1,
            0x01, 0x23, 0x45, 0x67,
            0x89, 0xAB, 0xCD, 0xEF
        });
    THROW_END
}


TEST(test_rtp_packet, bitstream)
{
    NO_THROW_BEGIN
        std::string bitstream(
            "\xB3\x96\xAC\xE1"
            "\x01\x23\x45\x67"
            "\x89\xAB\xCD\xEF"
            "\x00\x00\x10\x00"
            "\x00\x00\x20\x00"
            "\x00\x00\x30\x00",
            24
        );
        packet_t::header header(bitstream.begin(), bitstream.end());
        header.contributing_sources.push_back(0x00001000);
        header.contributing_sources.push_back(0x00002000);
        header.contributing_sources.push_back(0x00003000);
        auto out_bitstream = header.bitstream();
        ASSERT_EQ(out_bitstream.size(), 24);
        for (size_t i = 0; i < 24; i++)
            EXPECT_EQ(out_bitstream[i], bitstream[i]);
    NO_THROW_END
}
