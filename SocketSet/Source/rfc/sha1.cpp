/**
 * Modified by TekuConcept on July 28, 2017
 * 
 * Source for C-implementation provided in RFC documentation.
 * Copyright (C) The Internet Society (2001). All Rights Reserved.
 */

#include "rfc/sha1.h"

#include "utils/impact_error.h"

using namespace impact;

#define circular_shift(bits,word) \
                (((word) << (bits)) | ((word) >> (32-(bits))))

const unsigned int sha1::HASH_SIZE = 20;


std::string
sha1::digest(std::string __message)
{
    struct context ctx;
    STATUS status;
    unsigned char message_digest[HASH_SIZE];
    
    status = _S_reset(&ctx);
    if (status != STATUS::SUCCESS)
        throw impact_error("something unexpected happened"); // reset error

    status = _S_input(&ctx, (const unsigned char*)__message.c_str(),
        __message.length());
    switch (status) {
    case STATUS::NIL:
    case STATUS::STATE_ERROR:    throw impact_error("internal error"); break;
    case STATUS::INPUT_TOO_LONG: throw impact_error("input too long"); break;
    default: /* success */ break;
    }
    
    status = _S_result(&ctx, (unsigned char*)message_digest);
    switch (status) {
    case STATUS::NIL:
    case STATUS::STATE_ERROR:    throw impact_error("internal error"); break;
    case STATUS::INPUT_TOO_LONG: throw impact_error("input too long"); break;
    default: /* success */ break;
    }
    
    return std::string((const char*)(message_digest), HASH_SIZE);
}


/**
 *  Reset
 *
 *  Description:
 *      This function will initialize the SHA1 Context in preparation
 *      for computing a new SHA1 message digest.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to reset.
 *
 *  Returns:
 *      sha Error Code.
 *
 */
sha1::STATUS
sha1::_S_reset(struct context* __context)
{
    if (!__context) return STATUS::NIL;

    __context->length_low           = 0;
    __context->length_high          = 0;
    __context->message_block_index  = 0;
    __context->intermediate_hash[0] = 0x67452301;
    __context->intermediate_hash[1] = 0xEFCDAB89;
    __context->intermediate_hash[2] = 0x98BADCFE;
    __context->intermediate_hash[3] = 0x10325476;
    __context->intermediate_hash[4] = 0xC3D2E1F0;
    __context->computed             = false;
    __context->corrupted            = false;

    return STATUS::SUCCESS;
}


/**
 *  Result
 *
 *  Description:
 *      This function will return the 160-bit message digest into the
 *      message digest array provided by the caller.
 *      NOTE: The first octet of the hash is stored in the 0th element,
 *            the last octet of the hash in the 19th element.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to use to calculate the SHA-1 hash.
 *      message_digest: [out]
 *          Where the digest is returned.
 *
 *  Returns:
 *      sha Error Code.
 *
 */
sha1::STATUS
sha1::_S_result(
    struct context* __context,
    unsigned char*  __message_digest)
{
    if (!__context || !__message_digest || __context->corrupted)
        return STATUS::NIL;

    if (!__context->computed) {
        _S_pad_message(__context);
        for (int i = 0; i < 64; i++) {
            /* message may be sensitive, clear it out */
            __context->message_block[i] = 0;
        }
        __context->length_low  = 0; /* and clear the length */
        __context->length_high = 0;
        __context->computed    = true;
    }

    for (unsigned int i = 0; i < HASH_SIZE; i++) {
        __message_digest[i] = (unsigned char)
            (__context->intermediate_hash[i >> 2] >> 8 * (3 - (i & 0x03)));
    }

    return STATUS::SUCCESS;
}


/**
 *  Input
 *
 *  Description:
 *      This function accepts an array of octets as the next portion
 *      of the message.
 *
 *  Parameters:
 *      context: [in/out]
 *          The SHA context to update
 *      message_array: [in]
 *          An array of characters representing the next portion of
 *          the message.
 *      length: [in]
 *          The length of the message in message_array
 *
 *  Returns:
 *      sha Error Code.
 *
 */
sha1::STATUS
sha1::_S_input(
    struct context*      __context,
    const unsigned char* __message_array,
    unsigned int         __length)
{
    if (!__length) return STATUS::SUCCESS;
    else if (!__context || !__message_array || __context->corrupted)
        return STATUS::NIL;
    else if (__context->computed) {
        __context->corrupted = true;
        return STATUS::STATE_ERROR;
    }
    
    while (__length-- && !__context->corrupted) {
        __context->message_block[__context->message_block_index++] =
                        (*__message_array & 0xFF);
    
        __context->length_low += 8;
        if (__context->length_low == 0) {
            __context->length_high++;
            if (__context->length_high == 0) {
                /* Message is too long */
                __context->corrupted = true;
            }
        }
    
        if (__context->message_block_index == 64)
            _S_process_message_block(__context);
    
        __message_array++;
    }

    return STATUS::SUCCESS;
}


/**
 *  ProcessMessageBlock
 *
 *  Description:
 *      This function will process the next 512 bits of the message
 *      stored in the Message_Block array.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      Many of the variable names in this code, especially the
 *      single character names, were used because those were the
 *      names used in the publication.
 *
 *
 */
void
sha1::_S_process_message_block(struct context* __context)
{
    const unsigned int K[] = {  /* Constants defined in SHA-1  */
        0x5A827999,
        0x6ED9EBA1,
        0x8F1BBCDC,
        0xCA62C1D6
    };
    int t;                      /* Loop counter                */
    unsigned int temp;          /* Temporary word value        */
    unsigned int W[80];         /* Word sequence               */
    unsigned int A, B, C, D, E; /* Word buffers                */

    /**
     *  Initialize the first 16 words in the array W
     */
    for (t = 0; t < 16; t++) {
        W[t]  = __context->message_block[t * 4    ] << 24;
        W[t] |= __context->message_block[t * 4 + 1] << 16;
        W[t] |= __context->message_block[t * 4 + 2] << 8;
        W[t] |= __context->message_block[t * 4 + 3];
    }

    for (t = 16; t < 80; t++) {
       W[t] = circular_shift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
    }

    A = __context->intermediate_hash[0];
    B = __context->intermediate_hash[1];
    C = __context->intermediate_hash[2];
    D = __context->intermediate_hash[3];
    E = __context->intermediate_hash[4];

    for (t = 0; t < 20; t++) {
        temp = circular_shift(5,A) + ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        E = D;
        D = C;
        C = circular_shift(30,B);
        B = A;
        A = temp;
    }

    for (t = 20; t < 40; t++) {
        temp = circular_shift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        E = D;
        D = C;
        C = circular_shift(30,B);
        B = A;
        A = temp;
    }

    for (t = 40; t < 60; t++) {
        temp = circular_shift(5,A) +
            ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        E = D;
        D = C;
        C = circular_shift(30,B);
        B = A;
        A = temp;
    }

    for (t = 60; t < 80; t++) {
        temp = circular_shift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        E = D;
        D = C;
        C = circular_shift(30,B);
        B = A;
        A = temp;
    }

    __context->intermediate_hash[0] += A;
    __context->intermediate_hash[1] += B;
    __context->intermediate_hash[2] += C;
    __context->intermediate_hash[3] += D;
    __context->intermediate_hash[4] += E;
    __context->message_block_index = 0;
}


/**
 *  PadMessage
 *
 *  Description:
 *      According to the standard, the message must be padded to an even
 *      512 bits.  The first padding bit must be a '1'.  The last 64
 *      bits represent the length of the original message.  All bits in
 *      between should be 0.  This function will pad the message
 *      according to those rules by filling the Message_Block array
 *      accordingly.  It will also call the ProcessMessageBlock function
 *      provided appropriately.  When it returns, it can be assumed that
 *      the message digest has been computed.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to pad
 *      ProcessMessageBlock: [in]
 *          The appropriate SHA*ProcessMessageBlock function
 *  Returns:
 *      Nothing.
 *
 */
void
sha1::_S_pad_message(struct context* __context)
{
    /**
     *  Check to see if the current message block is too small to hold
     *  the initial padding bits and length.  If so, we will pad the
     *  block, process it, and then continue padding into a second
     *  block.
     */
    if (__context->message_block_index > 55) {
        __context->message_block[__context->message_block_index++] = 0x80;
        while (__context->message_block_index < 64)
            __context->message_block[__context->message_block_index++] = 0;

        _S_process_message_block(__context);

        while (__context->message_block_index < 56)
            __context->message_block[__context->message_block_index++] = 0;
    }
    else {
        __context->message_block[__context->message_block_index++] = 0x80;
        while (__context->message_block_index < 56)
            __context->message_block[__context->message_block_index++] = 0;
    }

    // Store the message length as the last 8 octets
    for (unsigned short i = 0, j; i < 4; i++) {
        j = 24 - (8 * i);
        __context->message_block[56+i] =
            (unsigned char)(__context->length_high >> j);
        __context->message_block[60+i] =
            (unsigned char)(__context->length_low  >> j);
    }

    _S_process_message_block(__context);
}
