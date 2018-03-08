/**
 * Modified by TekuConcept on July 28, 2017
 * 
 * Source for C-implementation provided in RFC documentation.
 * Copyright (C) The Internet Society (2001). All Rights Reserved.
 */

#include "RFC/SHA1.h"
#include <exception>

using namespace Impact;

#define circularShift(bits,word) \
                (((word) << (bits)) | ((word) >> (32-(bits))))

const unsigned int SHA1::HASH_SIZE = 20;

SHA1::SHA1() {}

std::string SHA1::digest(std::string message) {
    Context sha;
    int err;
    unsigned char messageDigest[HASH_SIZE];
    
    err = reset(&sha);
    if(err) throw std::exception(); // reset error (shouldn't ever throw)

    err = input(&sha, (const unsigned char*)message.c_str(), message.length());
    if(err) throw std::exception(); // input error

    err = result(&sha, (unsigned char*)messageDigest);
    if(err) throw std::exception();
    
    std::string result(
        reinterpret_cast<const char*>(messageDigest), HASH_SIZE);
    return result;
}

/*
 *  SHA1Reset
 *
 *  Description:
 *      This function will initialize the SHA1Context in preparation
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
SHA1::SHA_STATE SHA1::reset(Context *context) {
    if (!context) return SHA_STATE::Null;

    context->LengthLow             = 0;
    context->LengthHigh            = 0;
    context->MessageBlockIndex     = 0;

    context->IntermediateHash[0]   = 0x67452301;
    context->IntermediateHash[1]   = 0xEFCDAB89;
    context->IntermediateHash[2]   = 0x98BADCFE;
    context->IntermediateHash[3]   = 0x10325476;
    context->IntermediateHash[4]   = 0xC3D2E1F0;

    context->Computed   = false;
    context->Corrupted  = false;

    return SHA_STATE::Success;
}

/*
 *  SHA1Result
 *
 *  Description:
 *      This function will return the 160-bit message digest into the
 *      Message_Digest array  provided by the caller.
 *      NOTE: The first octet of hash is stored in the 0th element,
 *            the last octet of hash in the 19th element.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to use to calculate the SHA-1 hash.
 *      Message_Digest: [out]
 *          Where the digest is returned.
 *
 *  Returns:
 *      sha Error Code.
 *
 */
SHA1::SHA_STATE SHA1::result(Context *context, unsigned char* messageDigest) {
    if (!context || !messageDigest || context->Corrupted)
        return SHA_STATE::Null;
    if (!context->Computed) {
        padMessage(context);
        for(int i = 0; i < 64; i++){
            /* message may be sensitive, clear it out */
            context->MessageBlock[i] = 0;
        }
        context->LengthLow = 0;    /* and clear length */
        context->LengthHigh = 0;
        context->Computed = true;
    }

    for(unsigned int i = 0; i < HASH_SIZE; i++) {
        messageDigest[i] = (unsigned char)(
			context->IntermediateHash[i>>2] >> 8 * (3 - (i & 0x03)));
    }

    return SHA_STATE::Success;
}

/*
 *  SHA1Input
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
SHA1::SHA_STATE SHA1::input(Context *context, const unsigned char *messageArray,
    unsigned int length) {
    if (!length) return SHA_STATE::Success;
    else if (!context || !messageArray || context->Corrupted)
        return SHA_STATE::Null;
    else if (context->Computed) {
        context->Corrupted = true;
        return SHA_STATE::StateError;
    }
    
    while(length-- && !context->Corrupted) {
        context->MessageBlock[context->MessageBlockIndex++] =
                        (*messageArray & 0xFF);
    
        context->LengthLow += 8;
        if (context->LengthLow == 0) {
            context->LengthHigh++;
            if (context->LengthHigh == 0) {
                /* Message is too long */
                context->Corrupted = true;
            }
        }
    
        if (context->MessageBlockIndex == 64)
            processMessageBlock(context);
    
        messageArray++;
    }

    return SHA_STATE::Success;
}

/*
 *  SHA1ProcessMessageBlock
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
void SHA1::processMessageBlock(Context *context) {
    const unsigned int K[] = {       /* Constants defined in SHA-1  */
                            0x5A827999,
                            0x6ED9EBA1,
                            0x8F1BBCDC,
                            0xCA62C1D6
                            };
    int           t;                 /* Loop counter                */
    unsigned int  temp;              /* Temporary word value        */
    unsigned int  W[80];             /* Word sequence               */
    unsigned int  A, B, C, D, E;     /* Word buffers                */

    /*
     *  Initialize the first 16 words in the array W
     */
    for(t = 0; t < 16; t++) {
        W[t]  = context->MessageBlock[t * 4] << 24;
        W[t] |= context->MessageBlock[t * 4 + 1] << 16;
        W[t] |= context->MessageBlock[t * 4 + 2] << 8;
        W[t] |= context->MessageBlock[t * 4 + 3];
    }

    for(t = 16; t < 80; t++) {
       W[t] = circularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
    }

    A = context->IntermediateHash[0];
    B = context->IntermediateHash[1];
    C = context->IntermediateHash[2];
    D = context->IntermediateHash[3];
    E = context->IntermediateHash[4];

    for(t = 0; t < 20; t++) {
        temp =  circularShift(5,A) +
                ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        E = D;
        D = C;
        C = circularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 20; t < 40; t++) {
        temp = circularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        E = D;
        D = C;
        C = circularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 40; t < 60; t++) {
        temp = circularShift(5,A) +
               ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        E = D;
        D = C;
        C = circularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 60; t < 80; t++) {
        temp = circularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        E = D;
        D = C;
        C = circularShift(30,B);
        B = A;
        A = temp;
    }

    context->IntermediateHash[0] += A;
    context->IntermediateHash[1] += B;
    context->IntermediateHash[2] += C;
    context->IntermediateHash[3] += D;
    context->IntermediateHash[4] += E;

    context->MessageBlockIndex = 0;
}

/*
 *  SHA1PadMessage
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
void SHA1::padMessage(Context *context) {
    /*
     *  Check to see if the current message block is too small to hold
     *  the initial padding bits and length.  If so, we will pad the
     *  block, process it, and then continue padding into a second
     *  block.
     */
    if (context->MessageBlockIndex > 55) {
        context->MessageBlock[context->MessageBlockIndex++] = 0x80;
        while(context->MessageBlockIndex < 64)
            context->MessageBlock[context->MessageBlockIndex++] = 0;

        processMessageBlock(context);

        while(context->MessageBlockIndex < 56)
            context->MessageBlock[context->MessageBlockIndex++] = 0;
    }
    else {
        context->MessageBlock[context->MessageBlockIndex++] = 0x80;
        while(context->MessageBlockIndex < 56)
            context->MessageBlock[context->MessageBlockIndex++] = 0;
    }

    // Store the message length as the last 8 octets
    for(unsigned short i = 0, j; i < 4; i++) {
        j = 24 - (8 * i);
        context->MessageBlock[56+i] = (unsigned char)(context->LengthHigh >> j);
        context->MessageBlock[60+i] = (unsigned char)(context->LengthLow  >> j);
    }

    processMessageBlock(context);
}