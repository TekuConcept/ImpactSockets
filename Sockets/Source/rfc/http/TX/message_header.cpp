/**
 * Created by TekuConcept on December 28, 2018
 */

#include "rfc/http/TX/message_header.h"

using namespace impact;
using namespace http;


message_header::message_header(
    std::string __name,
    std::string __value)
: m_field_name_(__name), m_field_value_(__value)
{
    // validate name
    // validate value
}


message_header::~message_header()
{}


const std::string&
message_header::field_name() const noexcept
{
    return m_field_name_;
}


const std::string&
message_header::field_value() const noexcept
{
    return m_field_value_;
}
