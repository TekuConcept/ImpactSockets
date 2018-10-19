/**
 * Created by TekuConcept on July 31, 2018
 */

#ifndef _IMPACT_ARCHITECTURE_H_
#define _IMPACT_ARCHITECTURE_H_

#include <cstdint>
#include <vector>

#include "utils/environment.h"

#if defined(__OS_WINDOWS__)
    #include <intrin.h>
#endif

namespace impact {
namespace internal {
    UNUSED_FUNCTION(inline int popcount_32(uint32_t T);)
    UNUSED_FUNCTION(inline int popcount_64(uint64_t T);)
    UNUSED_FUNCTION(inline uint8_t bit_swap(uint8_t T);)

    inline uint16_t byte_swap_16(uint16_t T);
    inline uint32_t byte_swap_32(uint32_t T);
    inline uint64_t byte_swap_64(uint64_t T);

    inline void to_big_endian_16(uint16_t& value);
    inline void to_big_endian_32(uint32_t& value);
    inline void to_big_endian_64(uint64_t& value);

    inline void to_little_endian_16(uint16_t& value);
    inline void to_little_endian_32(uint32_t& value);
    inline void to_little_endian_64(uint64_t& value);

    /* - - - - - - - - - - - - - - - - - - - - - - - - - - *\
    |  Inline Implementations                               |
    \* - - - - - - - - - - - - - - - - - - - - - - - - - - */

    UNUSED_FUNCTION(inline int popcount_32(uint32_t T) {
    #if defined(__OS_WINDOWS__)
        return __popcnt(T);
    #else
        return __builtin_popcount(T);
    #endif
    })

    UNUSED_FUNCTION(inline int popcount_64(uint64_t T) {
    #if defined(__OS_WINDOWS__)
        // NOTE: might need to check compatibility with Win32
        // and opt for two calls to __popcnt(T) instead
        return __popcnt64(T);
    #else
        return __builtin_popcountll(T);
    #endif
    })

    UNUSED_FUNCTION(inline uint8_t bit_swap(uint8_t T) {
        uint8_t S = ((T & 0xF0) >> 4) | ((T & 0x0F) << 4);
                S = ((S & 0xCC) >> 2) | ((S & 0x33) << 2);
        return     (((S & 0xAA) >> 1) | ((S & 0x55) << 1));
    })

    inline uint16_t byte_swap_16(uint16_t T) {
        return (((T & 0x00FF) << 8) | ((T & 0xFF00) >> 8));
    }

    inline uint32_t byte_swap_32(uint32_t T) {
        return (byte_swap_16(T >> 16) | ((byte_swap_16(T & 0x0000FFFF)) << 16));
    }

    inline uint64_t byte_swap_64(uint64_t T) {
        uint64_t b = byte_swap_32(T & 0x00000000FFFFFFFF);
        return (byte_swap_32(T >> 32) | (b << 32));
    }

    inline void to_big_endian_16(uint16_t& value) {
    #if defined(LITTLE_ENDIAN)
        value = byte_swap_16(value);
    #else
        (void)value;
    #endif
    }

    inline void to_big_endian_32(uint32_t& value) {
    #if defined(LITTLE_ENDIAN)
        value = byte_swap_32(value);
    #else
        (void)value;
    #endif
    }

    inline void to_big_endian_64(uint64_t& value) {
    #if defined(LITTLE_ENDIAN)
        value = byte_swap_64(value);
    #else
        (void)value;
    #endif
    }

    inline void to_little_endian_16(uint16_t& value) {
    #if defined(BIG_ENDIAN)
        value = byte_swap_16(value);
    #else
        (void)value;
    #endif
    }

    inline void to_little_endian_32(uint32_t& value) {
    #if defined(BIG_ENDIAN)
        value = byte_swap_32(value);
    #else
        (void)value;
    #endif
    }

    inline void to_little_endian_64(uint64_t& value) {
    #if defined(BIG_ENDIAN)
        value = byte_swap_64(value);
    #else
        (void)value;
    #endif
    }
}}

#endif
