/**
 * Created by TekuConcept on December 28, 2018
 */

#include <sstream>
#include "rfc/http/TX/response_message.h"

using namespace impact;

http::response_message::response_message()
: message(message_type::RESPONSE)
{}


http::response_message::~response_message()
{}


std::shared_ptr<http::response_message>
http::response_message::create(
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
http::response_message::code() const noexcept
{
    return m_code_;
}


const std::string&
http::response_message::status() const noexcept
{
    return m_status_;
}
