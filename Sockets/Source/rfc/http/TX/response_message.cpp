/**
 * Created by TekuConcept on December 28, 2018
 */

#include <iomanip>
#include <sstream>
#include "rfc/http/TX/response_message.h"

using namespace impact;
using namespace http;

response_message::response_message()
: message()
{}


response_message::~response_message()
{}


std::shared_ptr<response_message>
response_message::create(
    int         __code,
    std::string __status)
{
    std::shared_ptr<response_message> result =
        std::shared_ptr<response_message>(new response_message());
    result->m_code_   = __code;
    result->m_status_ = __status;
    return result;
}


int
response_message::code() const noexcept
{
    return m_code_;
}


const std::string&
response_message::status() const noexcept
{
    return m_status_;
}


message_type
response_message::type() const
{
    return message_type::RESPONSE;
}


std::string
response_message::_M_start_line()
{
    std::ostringstream os;
    os << "HTTP/" << m_http_major_ << "." << m_http_minor_ << " ";
    os << std::setfill('0') << std::setw(3);
    os << m_code_ << std::setfill(' ') << " " << m_status_;
    return os.str();
}
