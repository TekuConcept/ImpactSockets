/**
 * Created by TekuConcept on December 15, 2018
 */

#ifndef _IMPACT_HTTP_PARSER_
#define _IMPACT_HTTP_PARSER_

#include <cstdlib>
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <functional>
#include <vector>
#include <set>
#include "utils/case_string.h"
#include "utils/impact_error.h"
#include "rfc/http/types.h"

#include "rfc/http/TX/transfer_encoding.h"

namespace impact {
namespace http {

    // void
    // send(
    //     std::ostream&         __stream,
    //     const message_traits& __traits)
    // {
    //     if (__traits.type == message_type::REQUEST) {
    //         __stream << "GET / HTTP/1.1\r\n";
    //         __stream << "Transfer-Encoding: chunked\r\n";
    //         __stream << "\r\n";
    //         __stream << std::hex << __traits.data.size() << std::dec << "\r\n";
    //         __stream << __traits.data << "\r\n";
    //         __stream << "0\r\n";
    //         __stream << "\r\n";
    //     }
    //     else {
    //         __stream << "HTTP/1.1 200 OK\r\n";
    //         __stream << "\r\n";
    //     }
    // }
    
    // struct start_line;
    
    // namespace internal {
    //     int parse_start_line(const std::string& input,
    //         struct start_line* result);
    //     int parse_header_line(const std::string& input,
    //         std::pair<std::string,std::string>* result);
    // }
    
    
    // typedef enum class message_type {
    //     REQUEST, RESPONSE
    // } MessageType;
    
    
    // typedef struct start_line {
    //     message_type type;
    //     union {
    //         std::string method;
    //         std::string message;
    //     };
    //     union {
    //         std::string target;
    //         int status;
    //     };
    //     char http_major;
    //     char http_minor;
        
    //     start_line();
    //     start_line(const start_line&);
    //     ~start_line();
    // } StartLine;
    
    
    // // class transfer_encoding {
    // // public:
    // //     static virtual std::string name() = 0;
    // //     static virtual void encode() = 0;
    // //     static virtual void decode() = 0;
    // // };
    
    
    // // class chuncked : public transfer_encoding {
    // // public:
    // //     static virtual std::string name();
    // //     static virtual void encode();
    // //     static virtual void decode();
    // // };
    
    // typedef enum class body_format_type {
    //     FIXED, DYNAMIC
    // } BodyFormatType;
    
    
    // class body_format {
    // public:
    //     body_format();
    //     virtual ~body_format();
    //     body_format_type type() const noexcept;
    // protected:
    //     body_format_type m_type_;
    // };
    
    
    // class fixed_body : public body_format {
    // public:
    //     fixed_body();
    //     fixed_body(std::string data);
    //     virtual ~fixed_body();
    //     std::string data;
    // };
    
    
    // class transfer_encoding {
    // public:
    //     virtual ~transfer_encoding();
    //     // virtual void encode(const std::string& source,
    //     //     std::string* destination) = 0;
    //     // virtual void decode(const std::string& source,
    //     //     std::string* destination) = 0;
    // };
    
    
    // class dynamic_body : public body_format {
    // public:
    //     dynamic_body();
    //     dynamic_body(std::iostream* data_stream,
    //         std::vector<transfer_encoding*> encodings);
    //     virtual ~dynamic_body();
    //     std::iostream* data_stream;
    // private:
        
    // };
    
    
    // class message {
    // public:
    //     virtual ~message();
        
    //     // enforce abstract class type
    //     virtual message_type type() const noexcept = 0;
    //     int http_major() const noexcept;
    //     int http_minor() const noexcept;
        
    //     void send(std::ostream*) const;
        
    //     std::map<case_string,std::string> headers;
    //     std::shared_ptr<body_format> body;
        
    // protected:
    //     struct start_line m_start_line_;
    // };
    
    
    // class request_message : public message {
    // public:
    //     request_message(std::string method, std::string target);
    //     virtual ~request_message();
    //     virtual message_type type() const noexcept;
    //     std::string method() const noexcept;
    //     std::string target() const noexcept;
    // };
}}

#endif
