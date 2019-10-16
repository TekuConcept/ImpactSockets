/**
 * Created by TekuConcept on October 14, 2019
 */

#include "rfc/rtp/packet.h"

#include <iterator>
#include <algorithm>

#include "utils/impact_error.h"

using namespace impact;
using namespace rtp;


packet_t::header::header()
: version(2),
  padding(0),
  extension(0),
  contributing_source_count(0),
  marker(0),
  payload_type(0),
  sequence_number(0),
  timestamp(0),
  synchronization_source(0)
{ }


packet_t::header::header(std::array<unsigned char, 12> __bitstream)
: version(2)
{
    if (((__bitstream[0] & 0xC0) >> 6) != 2)
        throw impact_error("RTP header version not supported");

    // Note: must preserve byte order;
    // Network byte order is used on-wire, which may
    // differ from system byte order. Bit operators are
    // a universal / system-independent way of
    // transforming between byte orders
    this->padding                   = (__bitstream[0] & 0x20) >> 5;
    this->extension                 = (__bitstream[0] & 0x10) >> 4;
    this->contributing_source_count = (__bitstream[0] & 0x0F);
    this->marker                    = (__bitstream[1] & 0x80) >> 7;
    this->payload_type              = (__bitstream[1] & 0x7F);
    this->sequence_number = (__bitstream[2] << 8) | __bitstream[3];
    this->timestamp =
        (__bitstream[4]  << 24) |
        (__bitstream[5]  << 16) |
        (__bitstream[6]  <<  8) |
        (__bitstream[7]  <<  0);
    this->synchronization_source =
        (__bitstream[8]  << 24) |
        (__bitstream[9]  << 16) |
        (__bitstream[10] <<  8) |
        (__bitstream[11] <<  0);
}


packet_t::header::header(
    std::string::const_iterator __begin,
    std::string::const_iterator __end)
: version(2)
{
    if (std::distance(__begin, __end) < 12)
        throw impact_error("insufficient data");
    if ((((*__begin) & 0xC0) >> 6) != 2)
        throw impact_error("RTP header version not supported");

    // Note: must preserve byte order;
    // Network byte order is used on-wire, which may
    // differ from system byte order. Bit operators are
    // a universal / system-independent way of
    // transforming between byte orders
    this->padding                   = ((*__begin) & 0x20) >> 5;
    this->extension                 = ((*__begin) & 0x10) >> 4;
    this->contributing_source_count = ((*__begin) & 0x0F);
    this->marker                    = ((*++__begin) & 0x80) >> 7;
    this->payload_type              = ((*__begin) & 0x7F);
    this->sequence_number =
        (((*++__begin) & 0xFF) << 8) | ((*++__begin) & 0xFF);
    this->timestamp =
        (((*++__begin) & 0xFF) << 24) |
        (((*++__begin) & 0xFF) << 16) |
        (((*++__begin) & 0xFF) <<  8) |
        (((*++__begin) & 0xFF) <<  0);
    this->synchronization_source =
        (((*++__begin) & 0xFF) << 24) |
        (((*++__begin) & 0xFF) << 16) |
        (((*++__begin) & 0xFF) <<  8) |
        (((*++__begin) & 0xFF) <<  0);
}


packet_t::header::header(const header& __other)
: version(__other.version),
  padding(__other.padding),
  extension(__other.extension),
  contributing_source_count(__other.contributing_source_count),
  marker(__other.marker),
  payload_type(__other.payload_type),
  sequence_number(__other.sequence_number),
  timestamp(__other.timestamp),
  synchronization_source(__other.synchronization_source),
  contributing_sources(__other.contributing_sources)
{ }


packet_t::header::header(header&& __other)
: version(__other.version),
  padding(__other.padding),
  extension(__other.extension),
  contributing_source_count(__other.contributing_source_count),
  marker(__other.marker),
  payload_type(__other.payload_type),
  sequence_number(__other.sequence_number),
  timestamp(__other.timestamp),
  synchronization_source(__other.synchronization_source),
  contributing_sources(std::move(__other.contributing_sources))
{ }


void
packet_t::header::operator=(const header& __other)
{
    padding = __other.padding;
    extension = __other.extension;
    contributing_source_count = __other.contributing_source_count;
    marker = __other.marker;
    payload_type = __other.payload_type;
    sequence_number = __other.sequence_number;
    timestamp = __other.timestamp;
    synchronization_source = __other.synchronization_source;
    contributing_sources = __other.contributing_sources;
}


void
packet_t::header::operator=(header&& __other)
{
    padding = __other.padding;
    extension = __other.extension;
    contributing_source_count = __other.contributing_source_count;
    marker = __other.marker;
    payload_type = __other.payload_type;
    sequence_number = __other.sequence_number;
    timestamp = __other.timestamp;
    synchronization_source = __other.synchronization_source;
    contributing_sources = std::move(__other.contributing_sources);
}


std::string
packet_t::header::bitstream() const
{
    std::string buffer;

    auto CC = std::min(contributing_sources.size(), 0xFUL);
    buffer.resize(12 + (CC << 2));

    buffer[0] =
        (version   << 6) |
        (padding   << 5) |
        (extension << 4) |
        (CC);
    buffer[1]  = (marker << 7) | payload_type;
    buffer[2]  = (sequence_number & 0xFF00) >> 8;
    buffer[3]  = (sequence_number & 0x00FF);
    buffer[4]  = (timestamp & 0xFF000000) >> 24;
    buffer[5]  = (timestamp & 0x00FF0000) >> 16;
    buffer[6]  = (timestamp & 0x0000FF00) >>  8;
    buffer[7]  = (timestamp & 0x000000FF);
    buffer[8]  = (synchronization_source & 0xFF000000) >> 24;
    buffer[9]  = (synchronization_source & 0x00FF0000) >> 16;
    buffer[10] = (synchronization_source & 0x0000FF00) >>  8;
    buffer[11] = (synchronization_source & 0x000000FF);
    for (size_t i = 0; i < CC; i++) {
        buffer[12 + ((i << 2) + 0)] =
            (contributing_sources[i] & 0xFF000000) >> 24;
        buffer[12 + ((i << 2) + 1)] =
            (contributing_sources[i] & 0x00FF0000) >> 16;
        buffer[12 + ((i << 2) + 2)] =
            (contributing_sources[i] & 0x0000FF00) >>  8;
        buffer[12 + ((i << 2) + 3)] =
            (contributing_sources[i] & 0x000000FF);
    }

    return buffer;
}
