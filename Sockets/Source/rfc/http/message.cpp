/**
 * Created by TekuConcept on August 9, 2018
 */

#include "rfc/http/message.h"

#include "utils/environment.h"

using namespace impact;
using namespace http;

message::message()
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


message
message::from_stream(std::istream* __stream)
{
    message result;
    if (!__stream) return result;
    
    std::vector<std::string> header;
    std::string line;
    char c;
    int trip = 0;
    
    while ((c = __stream->get()) != EOF) {
        if (c == '\r') trip++;
        else if (trip) {
            if (c == '\n') {
                trip = 0;
                if (line.size() == 0) break;
                header.push_back(line);
                line.clear();
            }
            else return result;
        }
        else line.push_back(c);
    }
    
    if (header.size()) {
        result.m_start_line_ = header[0];
        result.m_header_fields_.assign(header.begin() + 1, header.end());
    }
    
    while ((c = __stream->get()) != EOF) {
        result.m_message_body_.push_back(c);
    }
    
    // TODO
    return result;
}
