/**
 * Created by TekuConcept on August 9, 2018
 */

#ifndef _IMPACT_HTTP_URI_H_
#define _IMPACT_HTTP_URI_H_

#include <iostream>
#include <vector>
#include <string>
#include <memory>

namespace impact {
namespace http {
    typedef struct message_parser_opts {
        unsigned int line_size_limit    = 8000;
        unsigned int body_size_limit    = 200000;
        unsigned int header_count_limit = 101;
    } MessageParserOptions;
    
    
    typedef enum class message_type {
        UNKNOWN,
        REQUEST,
        RESPONSE,
    } MessageType;

    
    class message {
    public:
        message();
        virtual ~message();

        message_type type() const;
        int major() const;
        int minor() const;

        virtual std::string start_line() const;
        std::string message_body() const;
        std::vector<std::string> header_fields() const;

        static std::shared_ptr<message> from_stream(
            std::istream* stream, struct message_parser_opts* options = NULL);

    private:
        message_type m_type_;
        int m_major_;
        int m_minor_;
        
        // union {
        //     std::string method;
        //     std::string status_message;
        // }
        // union {
        //     unsigned int status_code;
        //     std::string request_target;
        // }
        
        std::string m_message_body_;
        std::string m_start_line_;
        std::vector<std::string> m_header_fields_;
        
        static inline bool _S_TCHAR(int);
        static bool _M_header_getline(std::istream*,std::string*,unsigned int);
        static bool _M_process_start_line(const std::string&,
            std::shared_ptr<message>*);
        
        friend class test_message_c; /* guts private access */
    };
    typedef std::shared_ptr<message> message_ptr;
    
    
    class request_message : public message {
    public:
        request_message();
        ~request_message();
    private:
        std::string m_method_;
        std::string m_request_target_;
    };
    
    
    class response_message : public message {
    public:
        response_message();
        ~response_message();
    private:
        // int m_status_code_;
        std::string m_reason_phrase_;
    };
}}

#endif