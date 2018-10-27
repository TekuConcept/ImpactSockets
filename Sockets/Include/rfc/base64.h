/**
 * Created by TekuConcept on July 25, 2017
 */

#ifndef _IMPACT_RFC_BASE64_H_
#define _IMPACT_RFC_BASE64_H_

#include <string>
#include <sstream>

#ifndef RFC4648
    #define RFC4648 1 /* Part 2 */
#endif

namespace impact {
    class base64 {
    public:
        static bool encode(const std::string& data, std::string* result);
        static bool decode(const std::string& data, std::string* result);

    private:
        static const std::string   k_alphabet;
        static const char          k_pad;
        static const unsigned char k_symbol_size;
        static const unsigned char k_symbol;
        static const unsigned char k_byte;

        base64();
        static unsigned char _S_reverse_lookup(const char);
        static void _S_run_encode(const std::string&,std::ostringstream*);
        static void _S_pad_stream(const std::string&,std::ostringstream*);
        static bool _S_run_decode(
            const std::string&  data,
            std::ostringstream* stream,
            unsigned short*     padding,
            unsigned short*     tally,
            unsigned int*       reg24);
        static bool _S_unpad_stream(
            std::ostringstream* stream,
            unsigned short*     padding,
            unsigned short*     tally,
            unsigned int*       reg24);
    };
}

#endif
