/**
 * Modified by TekuConcept on July 28, 2017
 * 
 * Source for C-implementation provided in RFC documentation.
 * Copyright (C) The Internet Society (2001). All Rights Reserved.
 */

#ifndef SHA1_H
#define SHA1_H

#include <string>

namespace Impact {
    class SHA1 {
    private:
        SHA1();
        static const unsigned int HASH_SIZE;
        typedef enum State {
            Success = 0,
            Null,
            InputTooLong,
            StateError
        } SHA_STATE;
        typedef struct Context {
            unsigned int IntermediateHash[5];
            unsigned int LengthLow;
            unsigned int LengthHigh;
            short int MessageBlockIndex;
            unsigned char MessageBlock[64];
            bool Computed;
            bool Corrupted;
        } Context;
        
        static void padMessage(Context*);
        static void processMessageBlock(Context*);
        static SHA_STATE reset(Context*);
        static SHA_STATE input(Context*, const unsigned char*, unsigned int);
        static SHA_STATE result(Context*, unsigned char*);

    public:
        static std::string digest(std::string message);
    };
}

#undef HashSize
#endif