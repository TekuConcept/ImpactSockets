/**
 * Created by TekuConcept on August 9, 2018
 */

#include "rfc/http/message.h"

#include <memory>

#define VERB(x) std::cout << x << std::endl

#include "utils/environment.h"
#include "utils/abnf_ops.h"

using namespace impact;
using namespace http;


inline bool
message::_S_TCHAR(int __c)
{
    return
        ((__c >= 'A') && (__c <= 'Z' )) ||
        ((__c >= 'a') && (__c <= 'z' )) ||
        ((__c >= '0') && (__c <= '9' )) ||
        ((__c >= '#') && (__c <= '\'')) || /* #, $, %, &, ' */
        ((__c >= '^') && (__c <= '`' )) || /* ^, _, ` */
        (__c == '!' ) ||
        (__c == '*' ) || (__c == '+' )  ||
        (__c == '-' ) || (__c == '.' )  ||
        (__c == '|' ) || (__c == '~' );
}


request_message::request_message()
{}


request_message::~request_message()
{}


response_message::response_message()
{}


response_message::~response_message()
{}


message::message()
: m_type_(message_type::UNKNOWN),
  m_major_(1), m_minor_(1)
{}


message::~message()
{}


message_type message::type() const { return m_type_; }
int message::major() const { return m_major_; }
int message::minor() const { return m_minor_; }

std::string message::start_line() const { return m_start_line_; }
std::string message::message_body() const { return m_message_body_; }
std::vector<std::string> message::header_fields() const { return m_header_fields_; }


message_ptr
message::from_stream(
    std::istream*               __stream,
    struct message_parser_opts* __opts)
{
    if (!__stream) return nullptr;
    struct message_parser_opts opts;
    if (__opts) opts = *__opts;
    
    // process start line
    message_ptr result;
    std::string start_line;
    if (!_M_header_getline(__stream, &start_line, opts.line_size_limit))
        return nullptr;
    if (!_M_process_start_line(start_line, &result))
        return nullptr;
    
    // process header lines
    std::vector<std::string> header;
    // unsigned int header_count     = 0;
    bool         found_header_end = false;
    std::string  line;
    do {
        if (!_M_header_getline(__stream, &line, opts.line_size_limit))
            return nullptr;
        if (line.size() == 0) {
            found_header_end = true;
            break;
        }
        // if (header_count == (opts.header_count_limit + 1))
        //     return nullptr;
        header.push_back(line);
        // header_count++;
    } while (true);
    if (!found_header_end) return nullptr;
    
    // process body
    unsigned int body_size = 0;
    int c;
    while ((c = __stream->get()) != EOF) {
        if (body_size == opts.body_size_limit)
            return nullptr;
        result->m_message_body_.push_back(c);
        body_size++;
    }
    result->m_start_line_ = start_line;
    result->m_header_fields_.assign(header.begin(), header.end());
    
    return result;
}


bool
message::_M_header_getline(
    std::istream* __stream,
    std::string*  __line,
    unsigned int  __line_size_limit)
{
    int c;
    __line->clear();

    while ((c = __stream->get()) != EOF) {
        if (c == '\r') break;
        if (c == '\n') return false;
        __line->push_back((char)c);
        // \r\n is included in the limit by subtracting 2
        if (__line->size() >= (__line_size_limit - 2))
            return false;
    }

    if (c ==  EOF) return false; // check EOF from loop
    c = __stream->get();
    if (c ==  EOF) return false; // check EOF from next
    if (c != '\n') return false;

    return true;
}


bool
message::_M_process_start_line(
    const std::string& __line,
    message_ptr*       __message)
{
    // request-line = method SP request-target SP HTTP-version     (A * HTTP/n.n)
    //  status-line = HTTP-version SP status-code SP reason-phrase (HTTP/n.n 000 )
    
    /* REQUIREMENTS
    - must not be empty
    - must not start with white space
    - must not end with white space
    - must not contain more than one white space char in a row
    */
    
    if (__line.size() < 12)               return false;
    if (__line[0] == ' ')                 return false;
    
    const std::string k_http_version_prefix = "HTTP/";
    
    std::vector<std::string> tokens;
    std::string next_token;
    for (char c : __line) {
        if (c == ' ') {
            if (tokens.size() >= 3) return false;
            tokens.push_back(next_token);
            next_token.clear();
        }
        else next_token.push_back(c);
    }
    if (tokens.size() != 3) return false;
    
    if (tokens[0].substr(0, k_http_version_prefix.size()) ==
        k_http_version_prefix) {
        
    }
    else {
        // validate method
    }
    
    // const std::string k_http_version_prefix = "HTTP/";
    // std::string tokens[3];
    // size_t current_index;
    // message_ptr result = nullptr;
    
    // int c;
    // for (size_t i = 0; i < 5; i++) {
    //     current_index = i;
    //     c = (int)(0x00FF & __line[i]);
    //     if (c == ' ') goto request_target;
    //     else tokens[0].push_back((char)c);
    //     if (c == k_http_version_prefix[i]) continue;
    //     else if (_S_TCHAR(c))              goto method;
    //     else                               return false;
    // }
    
    // { // HTTP-Version (continued)
    //     result = message_ptr(new response_message());
    //     c = __line[++current_index];
    //     if (!DIGIT(c)) return false;
    //     (*__message)->m_major_ = (c - '0');
    //     if (__line[++current_index] != '.') return false;
    //     c = __line[++current_index];
    //     if (!DIGIT(c)) return false;
    //     (*__message)->m_minor_ = (c - '0');
    //     if (__line[++current_index] != ' ') return false;
    // }
    
    // { // status-code
    // }
    
    // { // reason-phrase
    // }
    
    // goto done;
    
    // method: {
    //     char d;
    //     for (int i = )
    // }
    
    // request_target: {
    //     result = message_ptr(new request_message());
        
    // }
    
    // { // HTTP-version
    // }
    
    
    // std::string tokens[3];
    // unsigned int token_idx = 0;
    // for (char c : __line) {
    //     if (c == ' ') token_idx++;
    //     else tokens[token_idx].push_back(c);
    //     if (token_idx == 3) return false;
    // }
    // if (token_idx != 2) return false;
    
    // done:
    *__message = message_ptr(new message());
    return true;
}
