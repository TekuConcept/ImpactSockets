/**
 * Created by TekuConcept on August 9, 2018
 */

#include "rfc/http/message.h"

#include "utils/environment.h"

using namespace impact;
using namespace http;

message::message()
: m_valid_(false)
{}


message::~message()
{}


std::string
message::start_line() const
{
    return m_start_line_;
}


std::vector<std::string>
message::header_fields() const
{
    return m_header_fields_;
}


std::string
message::message_body() const
{
    return m_message_body_;
}


bool
message::valid() const
{
    return m_valid_;
}


message
message::from_stream(
    std::istream*               __stream,
    struct message_parser_opts* __opts)
{
    message result;
    if (!__stream) return result;
    
    struct message_parser_opts opts;
    if (__opts) opts = *__opts;
    
    std::vector<std::string> header;
    std::string line;
    char c;
    bool triggered            = false;
    bool found_header_tail    = false;
    unsigned int line_size    = 0;
    unsigned int body_size    = 0;
    unsigned int header_count = 0;
    
    while ((c = __stream->get()) != EOF) {
        if (c == '\r') triggered = true;
        else if (triggered) {
            if (c != '\n')
                return result;
            if (line.size() == 0) {
                found_header_tail = true;
                break;
            }
            if (header_count == opts.header_count_limit)
                return result;
            header.push_back(line);
            header_count++;
            line.clear();
            line_size = 0;
            triggered = false;
        }
        else {
            if (c == '\n')
                return result;
            if (line_size == opts.line_size_limit)
                return result;
            line.push_back(c);
            line_size++;
        }
    }
    
    if (!found_header_tail) return result;
    if (!header.size())     return result;
    
    while ((c = __stream->get()) != EOF) {
        if (body_size == opts.body_size_limit)
            return result;
        result.m_message_body_.push_back(c);
        body_size++;
    }
    
    result.m_start_line_ = header[0];
    result.m_header_fields_.assign(header.begin() + 1, header.end());
    result.m_valid_ = true;

    return result;
}
