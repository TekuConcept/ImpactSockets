/**
 * Created by TekuConcept on July 31, 2018
 */

#include <cstdint>
#include <gtest/gtest.h>
#include <utils/bit_ops.h>

using namespace impact;
using namespace internal;


TEST(test_bit_ops, generic) {
    // EXPECT_EQ(popcount_32(0xA0005001), 5);
    // EXPECT_EQ(popcount_64(0xA000000050000001), 5);
    // EXPECT_EQ(bit_swap(0xEA), 0x57);
    
    EXPECT_EQ(byte_swap_16(0x0102),             0x0201);
    EXPECT_EQ(byte_swap_32(0x01020304),         0x04030201);
    EXPECT_EQ(byte_swap_64(0x0102030405060708), 0x0807060504030201);
}
