/**
 * Created by TekuConcept on August 13, 2018
 */

#include "rfc/http/method.h"

using namespace impact;

std::string
http::method_str(method_id __id) noexcept
{
    switch (__id) {
    case method_id::CONNECT: return "CONNECT";
    case method_id::DELETE:  return "DELETE";
    case method_id::GET:     return "GET";
    case method_id::HEAD:    return "HEAD";
    case method_id::OPTIONS: return "OPTIONS";
    case method_id::POST:    return "POST";
    case method_id::PUT:     return "PUT";
    case method_id::TRACE:   return "TRACE";
    default: return "";
    }
}

bool
http::idempotent(method_id __id) noexcept
{
    switch (__id) {
    case method_id::CONNECT:
    case method_id::POST:    return false;
    case method_id::DELETE:
    case method_id::GET:
    case method_id::HEAD:
    case method_id::OPTIONS:
    case method_id::PUT:
    case method_id::TRACE:   return true;
    default:                 return false;
    }
}
