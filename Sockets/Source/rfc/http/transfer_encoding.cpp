/**
 * Created by TekuConcept on December 28, 2018
 */

#include <sstream>
#include "utils/impact_error.h"
#include "utils/environment.h"
#include "utils/abnf_ops.h"
#include "utils/string_ops.h"
#include "rfc/http/abnf_ops.h"
#include "rfc/http/transfer_encoding.h"

using namespace impact;
using namespace http;

#define VCHAR(c) impact::internal::VCHAR(c)


chunk_extension_token::chunk_extension_token(std::string __line)
{
    size_t index = __line.find_first_of("=");
    if (index != std::string::npos) {
        m_name_  = __line.substr(0, index);
        if (index == __line.size() - 1)
            throw impact_error("no value");
        m_value_ = __line.substr(index + 1, std::string::npos);
    }
    else std::swap(m_name_, __line);
    
    if (!internal::is_token(m_name_))
        throw impact_error("bad name \"" + m_name_ + "\"");
    if (m_value_.size()) {
        if (!internal::is_token(m_value_) &&
            !internal::is_quoted_string(m_value_))
            throw impact_error("bad value \"" + m_value_ + "\"");
    }
}


chunk_extension_token::chunk_extension_token(
    std::string __name,
    std::string __value)
{
    if (!internal::is_token(__name))
        throw impact_error("Bad name \"" + __name + "\"");
    if (__value.size()) { // empty value is allowed here
        if (!internal::is_token(__value) &&
            !internal::is_quoted_string(__value))
            throw impact_error("Bad value \"" + __value + "\"");
    }
    std::swap(m_name_, __name);
    std::swap(m_value_, __value);
}


chunk_extension_token::~chunk_extension_token()
{}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   transfer_encoding
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


std::map<case_string,std::shared_ptr<transfer_encoding>>
transfer_encoding::m_encodings_ = {
    { "chunked", transfer_encoding::chunked() }
};


transfer_encoding::transfer_encoding()
{}


transfer_encoding::transfer_encoding(std::string __name)
: transfer_encoding(__name, nullptr, nullptr)
{}


transfer_encoding::transfer_encoding(
    std::string      __name,
    codec_callback&& __encoder,
    codec_callback&& __decoder)
: m_encode_(__encoder), m_decode_(__decoder),
  m_name_(__name.c_str(), __name.size())
{
    for (char c : m_name_) {
        if (!VCHAR(c)) throw impact_error("Invalid name \"" + __name + "\"");
    }
    if (m_name_ == "chunked")
        throw impact_error("\"" + __name + "\" is reserved");
    
    if (m_encode_ == nullptr)
        m_encode_ = [](const std::string& data) -> std::string { return data; };
    if (m_decode_ == nullptr)
        m_decode_ = [](const std::string& data) -> std::string { return data; };
}


transfer_encoding::~transfer_encoding()
{}


std::shared_ptr<transfer_encoding>
transfer_encoding::chunked(
    extension_callback&& __extension_callback,
    trailer_callback&&   __trailer_callback)
{
    return std::shared_ptr<transfer_encoding>(
        (transfer_encoding*)(new chunked_encoding(
            std::move(__extension_callback),
            std::move(__trailer_callback)
        ))
    );
}


void
transfer_encoding::register_encoding(transfer_encoding_ptr __copy)
{
    if (__copy == nullptr) return;
    m_encodings_[__copy->name()] = __copy;
}


transfer_encoding_ptr
transfer_encoding::get_by_name(case_string __name) noexcept
{
    auto result = m_encodings_.find(__name);
    if (result == m_encodings_.end())
        return nullptr;
    else return result->second;
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
   chunked_encoding
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


chunked_encoding::chunked_encoding(
    extension_callback&& __extension_callback,
    trailer_callback&&   __trailer_callback)
: m_extension_callback_(__extension_callback),
  m_trailer_callback_(__trailer_callback)
{
    m_name_   = "chunked";
    m_encode_ =
    [&](const std::string& data) -> std::string {
        std::ostringstream os;
        chunk_extension_list* extension_list = NULL;
        if (m_extension_callback_ != nullptr)
            m_extension_callback_(&extension_list);
        
        os << std::hex << data.size() << std::dec;
        if (extension_list) {
            for (const auto& token : *extension_list) {
                os << ";" << token.name();
                if (token.value().size())
                    os << "=" << token.value();
            }
        }
        os << "\r\n";
        
        if (data.size())
            os << data << "\r\n";
        else {
            header_list* header_list = NULL;
            if (m_trailer_callback_)
                m_trailer_callback_(&header_list);
            if (header_list) {
                for (const auto& token : *header_list) {
                    os << token.name() << ": ";
                    os << token.value() << "\r\n";
                }
            }
            os << "\r\n";
        }
        
        return os.str();
    };
    m_decode_ =
    [&](const std::string& data) -> std::string {
        //  length := 0
        //  read chunk-size, chunk-ext (if any), and CRLF
        //  while (chunk-size > 0) {
        //     read chunk-data and CRLF
        //     append chunk-data to decoded-body
        //     length := length + chunk-size
        //     read chunk-size, chunk-ext (if any), and CRLF
        //  }
        //  read trailer field
        //  while (trailer field is not empty) {
        //     if (trailer field is allowed to be sent in a trailer) {
        //         append trailer field to existing header fields
        //     }
        //     read trailer-field
        //  }
        //  Content-Length := length
        //  Remove "chunked" from Transfer-Encoding
        //  Remove Trailer from existing header fields
        return data;
    };
}


chunked_encoding::~chunked_encoding()
{}


#include <iostream>
bool
chunked_encoding::decode_first_line(
    const std::string&                  __line,
    size_t*                             __chunk_size,
    std::vector<chunk_extension_token>* __extensions)
{
    if (__line.size() < 3) return false;
    if (__line[__line.size() - 1] != '\n' ||
        __line[__line.size() - 2] != '\r') return false;

    // maybe replace with string_ops::split(iterators)
    std::vector<std::string> tokens;
    size_t first = 0;
    size_t last  = __line.size() - 2;
    for (size_t next = first; next < last; next++) {
        if (__line[next] == '\\') next++; // escape next char
        else if (__line[next] == ';') {
            tokens.push_back(__line.substr(first, next - first));
            first = next + 1;
        }
    }
    if (first < last) tokens.push_back(__line.substr(first, last - first));
    
    try {
        size_t value = std::stoul(tokens[0], 0, 16);
        if (__chunk_size) *__chunk_size = value;
    }
    catch (std::out_of_range&) {
        if (__chunk_size) *__chunk_size = std::string::npos;
        return false;
    }
    catch (...) {
        if (__chunk_size) *__chunk_size = 0;
        return false;
    }
    
    try {
        for (size_t i = 1; i < tokens.size(); i++) {
            auto extension = chunk_extension_token(tokens[i]);
            if (__extensions) __extensions->push_back(extension);
        }
    }
    catch (...) { return false; }
    
    return true;
}
