/**
 * Created by TekuConcept on July 27, 2017
 */

#include "rfc/base64.h"

#include <stdexcept>

#include "utils/impact_error.h"
#include "utils/errno.h"

using namespace impact;

const std::string base64::k_alphabet =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char          base64::k_pad         = '=';
const unsigned char base64::k_symbol_size = 6;
const unsigned char base64::k_symbol      = 0x3F;
const unsigned char base64::k_byte        = 0xFF;


base64::base64()
{}


bool
base64::encode(
    const std::string& __data,
    std::string*       __result)
{
    imp_errno = imperr::SUCCESS;
    if (__data.length() == 0)
        return "";

    std::ostringstream os;

    _S_run_encode(__data, &os);
    _S_pad_stream(__data, &os);
    
    if (__result)
        *__result = os.str();
    return true;
}


void
base64::_S_run_encode(
    const std::string&  __data,
    std::ostringstream* __stream)
{
    unsigned int   reg24;
    unsigned short reg16[4];
    
    for (size_t i = 0; (i + 2) < __data.length(); i += 3) {
        reg24 = ((0xFF & __data[i    ]) << 16) |
                ((0xFF & __data[i + 1]) <<  8) |
                ((0xFF & __data[i + 2]));
        for (int j = 3; j >= 0; j--) {
            reg16[j] = reg24 & k_symbol;
            reg24 >>= k_symbol_size;
        }
        for (int j = 0; j < 4; j++)
            *__stream << k_alphabet[reg16[j]];
    }
}


void
base64::_S_pad_stream(
    const std::string&  __data,
    std::ostringstream* __stream)
{
    unsigned int reg24;
    unsigned short reg16[4];
    unsigned char pad_count =
        (__data.length() % 3 + (__data.length() + 1) % 3) - 1;
    
    if (pad_count != 0)  {
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
            *__stream << k_alphabet[reg16[i]];
        *__stream << padding;
    }
}


bool
base64::decode(
    const std::string& __data,
    std::string*       __result)
{
    imp_errno = imperr::SUCCESS;
    if (__data.length() == 0)
        return "";

    std::ostringstream stream;
    unsigned short     padding = 0;
    unsigned short     tally   = 0;
    unsigned int       reg24   = 0;
    
    // impact_errno: see run_decode()
    if (!_S_run_decode(
        __data,
        &stream,
        &padding,
        &tally,
        &reg24
    )) return false;
    
    // impact_errno: see unpad_stream
    if (!_S_unpad_stream(
        &stream,
        &padding,
        &tally,
        &reg24
    )) return false;

    if (__result)
        *__result = stream.str();
    
    return true;
}


bool
base64::_S_run_decode(
    const std::string&  __data,
    std::ostringstream* __stream,
    unsigned short*     __padding,
    unsigned short*     __tally,
    unsigned int*       __reg24)
{
    for (unsigned int i = 0; i < __data.length(); i++) {
        if (__data[i] == '=') {
            if ((__data.length() - i) <= 2)
                (*__padding)++;
            continue;
        }
        auto c = _S_reverse_lookup(__data[i]);
        if (c == '\x40') {
            imp_errno = imperr::B64_BADSYM;
            return false;
        }
        else {
            (*__reg24) |= c;
            (*__tally)++;
            if ((*__tally) == 4) {
                (*__stream) << (unsigned char)((*__reg24) >> 16);
                (*__stream) << (unsigned char)(((*__reg24) >> 8) & k_byte);
                (*__stream) << (unsigned char)((*__reg24) & k_byte);
                (*__tally) = (unsigned short)0;
                (*__reg24) = 0;
            }
            else (*__reg24) <<= k_symbol_size;
        }
    }
    return true;
}


bool
base64::_S_unpad_stream(
    std::ostringstream* __stream,
    unsigned short*     __padding,
    unsigned short*     __tally,
    unsigned int*       __reg24)
{
    if ((*__padding + *__tally) == 4) {
        *__reg24 <<= (k_symbol_size * (*__padding - 1));
        *__stream << (unsigned char)(*__reg24 >> 16);
        if (*__padding != 2)
            *__stream << (unsigned char)((*__reg24 >> 8) & k_byte);
    }
    else if ((*__padding + *__tally) != 0) {
        imp_errno = imperr::B64_BADPAD;
        return false;
    }
    return true;
}


unsigned char
base64::_S_reverse_lookup(const char __c)
{
    if      (__c >= 'A' && __c <= 'Z') return (unsigned char)(__c - 'A');
    else if (__c >= 'a' && __c <= 'z') return (unsigned char)(__c - 'a' + 26);
    else if (__c >= '0' && __c <= '9') return (unsigned char)(__c - '0' + 52);
    else if (__c == '+')               return (unsigned char)('\x3E');
    else if (__c == '/')               return (unsigned char)('\x3F');
    else                               return (unsigned char)('\x40');
}
