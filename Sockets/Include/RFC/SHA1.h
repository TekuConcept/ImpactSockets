/**
 * Modified by TekuConcept on July 28, 2017
 * 
 * Source for C-implementation provided in RFC documentation.
 * Copyright (C) The Internet Society (2001). All Rights Reserved.
 */

#ifndef SHA1_H
#define SHA1_H

#include <stdint.h>
#include <string>

#define HashSize 20

namespace Impact {
    class SHA1 {
    private:
        SHA1();
        typedef enum State {
            Success = 0,
            Null,            /* Null pointer parameter */
            InputTooLong,    /* input data too long */
            StateError       /* called Input after Result */
        } STATE;
        typedef struct Context {
            uint32_t Intermediate_Hash[HashSize/4];
            uint32_t Length_Low;
            uint32_t Length_High;
            int_least16_t Message_Block_Index;
            uint8_t Message_Block[64];
            int Computed;
            int Corrupted;
        } Context;
        
        static void padMessage(Context *);
        static void processMessageBlock(Context *);
        static int reset(Context *);
        static int input(Context *, const uint8_t *, unsigned int);
        static int result(Context *, uint8_t Message_Digest[HashSize]);

    public:
        static std::string digest(std::string message);
    };
}

#undef HashSize
#endif