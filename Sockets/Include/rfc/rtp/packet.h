/**
 * Created by TekuConcept on October 14, 2019
 */

#ifndef _IMPACT_RTP_PACKET_H_
#define _IMPACT_RTP_PACKET_H_

#include <array>
#include <vector>
#include <string>

namespace impact {
namespace rtp {

    class packet_t {
    public:
        /*
         0                   1                   2                   3
         0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |V=2|P|X|  CC   |M|     PT      |       sequence number         |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                           timestamp                           |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |           synchronization source (SSRC) identifier            |
        +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
        |            contributing source (CSRC) identifiers             |
        |                             ....                              |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        */
        struct header {
            const unsigned int version : 2;
            unsigned int padding : 1;
            unsigned int extension : 1;
            unsigned int contributing_source_count : 4;
            unsigned int marker : 1;
            unsigned int payload_type : 7;
            unsigned int sequence_number : 16;
            unsigned int timestamp : 32;
            unsigned int synchronization_source : 32;
            std::vector<unsigned int> contributing_sources;
            header();
            header(std::array<unsigned char, 12> bitstream);
            header(
                std::string::const_iterator begin,
                std::string::const_iterator end);
            header(const header&);
            header(header&&);
            void operator=(const header&);
            void operator=(header&&);
            std::string bitstream() const;
        };
    };

}}

#endif
