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
#include "rfc/http/types.h"

namespace impact {
namespace http {
    
    enum class message_type { REQUEST, RESPONSE };
    
    class transfer_encoding {
    public:
        transfer_encoding(std::string name,
            std::function<std::string(const std::string&)> encoder);
        ~transfer_encoding();
        const case_string& name() const noexcept;
        static transfer_encoding chunked();
        
    private:
        case_string m_name_;
        std::function<std::string(const std::string&)> m_encode_;
    
    public:
        inline std::string
        encode(const std::string& __) const
        { return m_encode_(__); }
        
        struct less {
            bool operator() (const transfer_encoding& lhs,
                const transfer_encoding& rhs) const;
        };
        
        typedef std::set<transfer_encoding, less> set;
    };
    
    class message_header {
    public:
        message_header(std::string name, std::string value);
        ~message_header();
        const std::string& field_name() const noexcept;
        const std::string& field_value() const noexcept;
    
    private:
        std::string m_field_name_;
        std::string m_field_value_;
    };
    
    class message {
    public:
        message() = delete;
        virtual ~message();
        message_type type() const noexcept;
        
        void send(std::ostream& stream);
    
    protected:
        message(message_type type);
        message(message_type type,
            transfer_encoding::set,
            std::function<void(std::string*)>);
        
    private:
        message_type m_type_;
        int m_http_major_;
        int m_http_minor_;
        
        std::vector<message_header> m_headers_;
        
        bool m_has_body_;
        bool m_is_fixed_body_;
        transfer_encoding::set m_transfer_encodings_;
        std::function<void(std::string*)> m_data_callback_;
        std::string m_data_buffer_;
    };
    
    class request_message : public message {
    public:
        virtual ~request_message();
        
        static std::shared_ptr<request_message>
            create(std::string method, std::string target);
        static std::shared_ptr<request_message>
            create(std::string method, std::string target,
            transfer_encoding::set encodings,
            std::function<void(std::string*)> data_callback);
        
        const std::string& method() const noexcept;
        const std::string& target() const noexcept;

    private:
        std::string m_method_;
        std::string m_target_;
        
        request_message();
        request_message(transfer_encoding::set,
            std::function<void(std::string*)>);
    };
    
    class response_message : public message {
    public:
        virtual ~response_message();
        
        static std::shared_ptr<response_message>
            create(int code, std::string status);
        
        int code() const noexcept;
        const std::string& status() const noexcept;
        
    private:
        int m_code_;
        std::string m_status_;
        
        response_message();
    };

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
