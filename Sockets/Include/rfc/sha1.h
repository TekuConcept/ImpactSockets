/**
 * Modified by TekuConcept on July 28, 2017
 * 
 * Source for C-implementation provided in RFC documentation.
 * Copyright (C) The Internet Society (2001). All Rights Reserved.
 */

#ifndef IMPACT_SHA1_H
#define IMPACT_SHA1_H

#include <string>

#define RFC3174 1

namespace impact {
    class sha1 {
    public:
        static std::string digest(std::string message);
    
    private:
        static const unsigned int HASH_SIZE;
        
        enum class STATUS {
            SUCCESS = 0,
            NIL,
            INPUT_TOO_LONG,
            STATE_ERROR
        };
        
        struct context {
            unsigned int intermediate_hash[5];
            unsigned int length_low;
            unsigned int length_high;
            short int message_block_index;
            unsigned char message_block[64];
            bool computed;
            bool corrupted;
        };
        
        static void _S_pad_message(struct context*);
        static void _S_process_message_block(struct context*);
        static STATUS _S_reset(struct context*);
        static STATUS _S_input(struct context*, const unsigned char*,
            unsigned int);
        static STATUS _S_result(struct context*, unsigned char*);
    };
}

#endif