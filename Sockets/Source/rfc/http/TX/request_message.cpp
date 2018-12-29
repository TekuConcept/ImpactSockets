/**
 * Created by TekuConcept on December 28, 2018
 */

#include <sstream>
#include "utils/impact_error.h"
#include "rfc/uri.h"
#include "rfc/http/abnf_ops.h"
#include "rfc/http/TX/request_message.h"

using namespace impact;
using namespace http;

method_token::method_token(std::string __method_name)
{
    if (!_M_valid_name(__method_name))
        throw impact_error("Invalid method name");
    m_name_ = __method_name;
}


method_token::~method_token()
{}


bool
method_token::_M_valid_name(const std::string& __method) const
{
    return internal::is_token(__method);
}


target_token::target_token(std::string __target_name)
{
    if ((m_type_ = _M_valid_target(__target_name)) == path_type::UNKNOWN)
        throw impact_error("Invalid target path");
    m_target_ = __target_name;
}


target_token::~target_token()
{}


target_token::path_type
target_token::_M_valid_target(const std::string& __target) const
{
    if (__target.size() == 0) return path_type::UNKNOWN;
    if (__target == "*") return path_type::ASTERISK;
    if (__target[0] == '/') {
        if (__target.size() == 1) return path_type::ORIGIN;
        // don't accidentally mistake authority "//"
        else if (__target[1] == '/') return path_type::UNKNOWN;
        else {
            if (uri::parse_resource(__target, NULL))
                 return path_type::ORIGIN;
            else return path_type::UNKNOWN;
        }
    }
    else {
        bool valid = uri::parse_authority(__target, NULL);
        if (!valid) {
            if (uri::parse(__target, NULL))
                 return path_type::ABSOLUTE;
            else return path_type::UNKNOWN;
        }
        else return path_type::AUTHORITY;
    }
}


request_message::request_message(
    method_token __method,
    target_token __target)
: message(message_type::REQUEST),
  m_method_(__method), m_target_(__target)
{}


request_message::request_message(
    method_token            __method,
    target_token            __target,
    transfer_encoding_token __data)
: message(message_type::REQUEST, __data),
  m_method_(__method), m_target_(__target)
{}


request_message::request_message(
    std::string __method,
    std::string __target)
: request_message(method_token(__method), target_token(__target))
{}


request_message::request_message(
    std::string                                     __method,
    std::string                                     __target,
    transfer_encoding_token::transfer_encoding_list __encodings,
    std::function<void(std::string*)>               __data_callback)
: request_message(method_token(__method), target_token(__target),
    transfer_encoding_token(__encodings, __data_callback))
{}


request_message::~request_message()
{}
