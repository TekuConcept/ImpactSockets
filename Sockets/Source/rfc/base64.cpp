/**
 * Created by TekuConcept on July 27, 2017
 */

#include "rfc/base64.h"

#include <sstream>
#include <stdexcept>

using namespace impact;

const std::string base64::k_alphabet =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char base64::k_pad = '=';


base64::base64()
{}


std::string
base64::encode(const std::string& __data)
{
    if (__data.length() == 0)
      return "";

    std::ostringstream os;
    const unsigned char k_symbol      = 0x3F;
    const unsigned char k_symbol_size = 6;

    // encode
    unsigned int   reg24;
    unsigned short reg16[4];
    for (unsigned int i = 0; (i + 2) < __data.length(); i += 3) {
        reg24 = ((0xFF & __data[i    ]) << 16) |
                ((0xFF & __data[i + 1]) <<  8) |
                ((0xFF & __data[i + 2]));
        for (short j = 3; j >= 0; j--) {
            reg16[j] = reg24 & k_symbol;
            reg24 >>= k_symbol_size;
        }
        for (unsigned char j = 0; j < 4; j++)
            os << k_alphabet[reg16[j]];
    }

    // determine padding
    unsigned char pad_count =
      (__data.length() % 3 + (__data.length() + 1) % 3) - 1;
    if (pad_count == 0)
      return os.str();
    else {
        std::string padding(pad_count, k_pad);
        reg24 = ((0xFF & __data[__data.length() - 3 + pad_count]) << 16);
        if (pad_count == 1)
          reg24 |= ((0xFF & __data[__data.length() - 1]) <<  8);
        else reg24 >>= k_symbol_size;
        for (short i = (3 - pad_count); i >= 0; i--) {
            reg24 >>= k_symbol_size;
            reg16[i] = reg24 & k_symbol;
        }
        for (short i = 0; i <= (3 - pad_count); i++)
            os << k_alphabet[reg16[i]];
        os << padding;
        return os.str();
    }
}

std::string
base64::decode(const std::string& __data)
{
    bool success;
    auto result = decode(__data, success);
    if (!success)
      throw std::runtime_error("base64::decode()\n");
    return result;
}


std::string
base64::decode(
  const std::string& __data,
  bool&              __status)
{
    __status = true;
    if (__data.length() == 0)
      return "";

    std::ostringstream os;
    const unsigned char
      k_byte        = 0xFF,
      k_symbol_size = 6;
    unsigned short
      padding       = 0,
      tally         = 0;
    unsigned int
      reg24         = 0;

    for (unsigned int i = 0; i < __data.length(); i++) {
        if (__data[i] == '=') {
            if ((__data.length() - i) <= 2)
              padding++;
            continue;
        }
        auto c = reverse_lookup(__data[i]);
        if (c == '\x40') {
            __status = false;
            break;
        }
        else {
            reg24 |= c;
            tally++;
            if (tally == 4) {
                os << (unsigned char)(reg24 >> 16);
                os << (unsigned char)((reg24 >> 8) & k_byte);
                os << (unsigned char)(reg24 & k_byte);
                tally = (unsigned short)0;
                reg24 = 0;
            }
            else reg24 <<= k_symbol_size;
        }
    }

    // take care of padded values
    if ((padding + tally) == 4) {
        reg24 <<= (k_symbol_size * (padding - 1));
        os << (unsigned char)(reg24 >> 16);
        if (padding != 2)
          os << (unsigned char)((reg24 >> 8) & k_byte);
    }
    else if ((padding + tally) != 0)
      __status = false;

    return os.str();
}


unsigned char
base64::reverse_lookup(const char __c)
{
    if      (__c >= 'A' && __c <= 'Z') return (unsigned char)(__c - 'A');
    else if (__c >= 'a' && __c <= 'z') return (unsigned char)(__c - 'a' + 26);
    else if (__c >= '0' && __c <= '9') return (unsigned char)(__c - '0' + 52);
    else if (__c == '+')               return (unsigned char)('\x3E');
    else if (__c == '/')               return (unsigned char)('\x3F');
    else                               return (unsigned char)('\x40');
}
