/**
 * Created by TekuConcept on December 15, 2018
 */

#ifndef _IMPACT_HTTP_TYPES_H_
#define _IMPACT_HTTP_TYPES_H_

#include <string>

namespace impact {
namespace http {
    enum class message_type {
        REQUEST, RESPONSE
    };
    
    typedef struct start_line {
        message_type type;
        union {
            std::string method;
            std::string message;
        };
        union {
            std::string target;
            int status;
        };
        char http_major;
        char http_minor;
        
        start_line();
        ~start_line();
    } StartLine;
}}

#endif
