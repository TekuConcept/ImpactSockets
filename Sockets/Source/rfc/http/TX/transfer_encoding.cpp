/**
 * Created by TekuConcept on December 28, 2018
 */

#include <sstream>
#include "utils/impact_error.h"
#include "utils/environment.h"
#include "rfc/http/abnf_ops.h"
#include "rfc/http/TX/transfer_encoding.h"

using namespace impact;
using namespace http;

#include <iostream>
#define VERBOSE(x) std::cout << x << std::endl


chunk_ext_token::chunk_ext_token(
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
    m_name_  = __name;
    m_value_ = __value;
}


chunk_ext_token::~chunk_ext_token()
{}


transfer_encoding::transfer_encoding()
{}


transfer_encoding::transfer_encoding(std::string __name)
: transfer_encoding(__name, nullptr)
{}


transfer_encoding::transfer_encoding(
    std::string      __name,
    encoder_callback __encoder)
: m_encode_(__encoder), m_name_(__name.c_str())
{
    if (m_name_ == "chunked")
        throw impact_error("\"" + __name + "\" is reserved");
}


transfer_encoding::~transfer_encoding()
{}


std::shared_ptr<transfer_encoding>
transfer_encoding::chunked(
    ext_callback     __extension_callback,
    trailer_callback __trailer_callback)
{
    return std::shared_ptr<transfer_encoding>(
        (transfer_encoding*)(new chunked_encoding(
            __extension_callback, __trailer_callback
        ))
    );
}


chunked_encoding::chunked_encoding(
    ext_callback     __ext_callback,
    trailer_callback __trailer_callback)
: m_ext_callback_(__ext_callback),
  m_trailer_callback_(__trailer_callback)
{
    m_name_   = "chunked";
    m_encode_ =
    [&](const std::string& data) -> std::string {
        std::ostringstream os;
        chunk_ext_list* extension_list = NULL;
        if (m_ext_callback_ != nullptr)
            m_ext_callback_(&extension_list);
        
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
            message_header_list* header_list = NULL;
            if (m_trailer_callback_)
                m_trailer_callback_(&header_list);
            if (header_list) {
                for (const auto& token : *header_list) {
                    os << token.field_name() << ": ";
                    os << token.field_value() << "\r\n";
                }
            }
            os << "\r\n";
        }
        
        return os.str();
    };
}


chunked_encoding::~chunked_encoding()
{}
