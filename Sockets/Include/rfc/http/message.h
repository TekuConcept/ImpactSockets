/**
 * Created by TekuConcept on August 9, 2018
 */

#ifndef _IMPACT_HTTP_URI_H_
#define _IMPACT_HTTP_URI_H_

#include <iostream>
#include <vector>

namespace impact {
namespace http {
    typedef struct message_parser_opts {
        unsigned int line_size_limit    = 8000;
        unsigned int body_size_limit    = 200000;
        unsigned int header_count_limit = 101;
    } MessageParserOptions;

    
    class message {
    public:
        message();
        virtual ~message();
        
        std::string start_line() const;
        std::vector<std::string> header_fields() const;
        std::string message_body() const;
        bool valid() const;
        
        static message from_stream(std::istream* stream,
            struct message_parser_opts* options = NULL);
        
    private:
        bool m_valid_;
        std::string m_start_line_;
        std::string m_message_body_;
        std::vector<std::string> m_header_fields_;
        
        friend class test_message_c; /* guts private access */
    };
}}

#endif