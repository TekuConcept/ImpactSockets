/**
 * Created by TekuConcept on January 30, 2019
 */

#include "rfc/md5.h"

using namespace impact;


#define MASK8 0xFF &
#if defined(HAVE_UINT32_T)
    #define MASK32
#else
    #define MASK32 0xFFFFFFFF &
#endif


md5::var md5::s[64] = {
    7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
    5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
    4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
    6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
};


md5::var md5::K[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};


std::string
md5::digest(std::string __message)
{
    _S_preprocess(&__message);
    return _S_process(__message);
}


void
md5::_S_preprocess(std::string* __message)
{
    auto& padded_message = *__message;
    size_t original_size = __message->size();
    
    // TODO: safe-guard against message overflow
    // max_size = size + roundup(size % 512)

    size_t zero_pad_length;
    size_t modulo1 = (original_size + 1) % 64;
    if (modulo1 <= 56) zero_pad_length =  56 - modulo1;
    else               zero_pad_length = 120 - modulo1;
    
    padded_message.append("\x80");
    padded_message.append(zero_pad_length + 8, '\0');

    original_size <<= 3; // get size in bits
    unsigned int sizeof_size_t = std::max(sizeof(size_t), (size_t)8U);
    char* trailer = &padded_message[padded_message.size() - sizeof_size_t];
    for (unsigned int i = 0; i < sizeof_size_t; i++)
        trailer[i] = 0xFF & (original_size >> (8 * i));
}


std::string
md5::_S_process(const std::string& __padded_message)
{
    var a0 = 0x67452301; // A
    var b0 = 0xefcdab89; // B
    var c0 = 0x98badcfe; // C
    var d0 = 0x10325476; // D

    for (size_t chunk_index = 0;
        chunk_index < __padded_message.size();
        chunk_index += 64)
    {
        // Break chunk into sixteen 32-bit words M[j], 0 ≤ j ≤ 15
        var M[16];
        for (unsigned int j = 0; j < 16; j++) {
            M[j] =
                ((MASK8 __padded_message[chunk_index + (j * 4) + 0]) <<  0) |
                ((MASK8 __padded_message[chunk_index + (j * 4) + 1]) <<  8) |
                ((MASK8 __padded_message[chunk_index + (j * 4) + 2]) << 16) |
                ((MASK8 __padded_message[chunk_index + (j * 4) + 3]) << 24);
        }
        
        var A = a0;
        var B = b0;
        var C = c0;
        var D = d0;
        
        // Main loop:
        for (var i = 0; i < 64; i++) {
            var F, j;
            if (0 <= i && i <= 15) {
                F = MASK32 ((B & C) | ((MASK32 (~B)) & D));
                j = i;
            }
            else if (16 <= i && i <= 31) {
                F = MASK32 ((B & D) | (C & (MASK32 (~D))));
                j = (5 * i + 1) % 16;
            }
            else if (32 <= i && i <= 47) {
                F = MASK32 (B ^ C ^ D);
                j = (3 * i + 5) % 16;
            }
            else /* if (48 <= i && i <= 63) */ {
                F = MASK32 (C ^ (B | (MASK32 (~D))));
                j = (7 * i) % 16;
            }
            
            // Be wary of the below definitions of a,b,c,d
            F = MASK32 (F + A + K[i] + M[j]);
            F = MASK32 ((F << s[i]) | (F >> (32 - s[i]))); // left-rotate
            A = D;
            D = C;
            C = B;
            B = MASK32 (B + F);
        }
        
        // Add this chunk's hash to result so far:
        a0 = MASK32 (a0 + A);
        b0 = MASK32 (b0 + B);
        c0 = MASK32 (c0 + C);
        d0 = MASK32 (d0 + D);
    }
    
    std::string result(16, '\0');
    result[ 0] = (char)(MASK8 (a0 >>  0));
    result[ 1] = (char)(MASK8 (a0 >>  8));
    result[ 2] = (char)(MASK8 (a0 >> 16));
    result[ 3] = (char)(MASK8 (a0 >> 24));
    result[ 4] = (char)(MASK8 (b0 >>  0));
    result[ 5] = (char)(MASK8 (b0 >>  8));
    result[ 6] = (char)(MASK8 (b0 >> 16));
    result[ 7] = (char)(MASK8 (b0 >> 24));
    result[ 8] = (char)(MASK8 (c0 >>  0));
    result[ 9] = (char)(MASK8 (c0 >>  8));
    result[10] = (char)(MASK8 (c0 >> 16));
    result[11] = (char)(MASK8 (c0 >> 24));
    result[12] = (char)(MASK8 (d0 >>  0));
    result[13] = (char)(MASK8 (d0 >>  8));
    result[14] = (char)(MASK8 (d0 >> 16));
    result[15] = (char)(MASK8 (d0 >> 24));
    return result;
}
