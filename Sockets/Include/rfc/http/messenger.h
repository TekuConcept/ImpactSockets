/**
 * Created by TekuConcept on December 15, 2018
 */

#ifndef _IMPACT_HTTP_PARSER_
#define _IMPACT_HTTP_PARSER_

#include <string>
#include "rfc/http/types.h"

namespace impact {
namespace http {
    struct start_line;
    
    namespace internal {
        int parse_start_line(const std::string& input,
            struct start_line* result);
        int parse_header_line(const std::string& input,
            std::pair<std::string,std::string>* result);
    }
    
    typedef enum class message_type {
        REQUEST, RESPONSE
    } MessageType;
    
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
    
    typedef struct message {
        
    } Message;
    
    struct message create_request();
}}

#endif
