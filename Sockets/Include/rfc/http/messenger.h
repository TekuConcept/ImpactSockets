/**
 * Created by TekuConcept on December 15, 2018
 */

#ifndef _IMPACT_HTTP_PARSER_
#define _IMPACT_HTTP_PARSER_

#include <iostream>
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
        start_line(const start_line&);
        ~start_line();
    } StartLine;
    
    
    class message {
    public:
        message();
        virtual ~message();
        message_type type() const noexcept;
        int http_major() const noexcept;
        int http_minor() const noexcept;
        
        void send(std::ostream*) const;
    protected:
        struct start_line __start_line;
    };
    
    
    class request_message : public message {
    public:
        request_message(std::string method, std::string target);
        virtual ~request_message();
        std::string method() const noexcept;
        std::string target() const noexcept;
    };
}}

#endif
