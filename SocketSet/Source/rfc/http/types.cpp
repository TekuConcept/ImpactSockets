/**
 * Created by TekuConcept on December 15, 2018
 */

#include "rfc/http/types.h"

using namespace impact;

std::string
http::method_string(method __id) noexcept
{
    switch (__id) {
    case method::CONNECT: return "CONNECT";
    case method::DELETE:  return "DELETE";
    case method::GET:     return "GET";
    case method::HEAD:    return "HEAD";
    case method::OPTIONS: return "OPTIONS";
    case method::POST:    return "POST";
    case method::PUT:     return "PUT";
    case method::TRACE:   return "TRACE";
    default: return "";
    }
}


bool
http::idempotent(method __id) noexcept
{
    switch (__id) {
    case method::CONNECT:
    case method::POST:    return false;
    case method::DELETE:
    case method::GET:
    case method::HEAD:
    case method::OPTIONS:
    case method::PUT:
    case method::TRACE:   return true;
    default:              return false;
    }
}


std::string
http::field_name_string(field_name __id) noexcept
{
    switch (__id) {
    case field_name::SET_COOKIE:          return "Set-Cookie";

    case field_name::CONNECTION:          return "Connection";
    case field_name::CONTENT_LENGTH:      return "Content-Length";
    case field_name::HOST:                return "Host";
    case field_name::TE:                  return "TE";
    case field_name::TRAILER:             return "Trailer";
    case field_name::TRANSFER_ENCODING:   return "Transfer-Encoding";
    case field_name::UPGRADE:             return "Upgrade";
    case field_name::VIA:                 return "Via";
    case field_name::CLOSE:               return "Close";
    
    case field_name::ACCEPT:              return "Accept";
    case field_name::ACCEPT_CHARSET:      return "Accept-Charset";
    case field_name::ACCEPT_ENCODING:     return "Accept-Encoding";
    case field_name::ACCEPT_LANGUAGE:     return "Accept-Language";
    case field_name::ALLOW:               return "Allow";
    case field_name::CONTENT_ENCODING:    return "Content-Encoding";
    case field_name::CONTENT_LANGUAGE:    return "Content-Language";
    case field_name::CONTENT_LOCATION:    return "Content-Location";
    case field_name::CONTENT_TYPE:        return "Content-Type";
    case field_name::DATE:                return "Date";
    case field_name::EXPECT:              return "Expect";
    case field_name::FROM:                return "From";
    case field_name::LOCATION:            return "Location";
    case field_name::MAX_FORWARDS:        return "Max-Forwards";
    case field_name::MIME_VERSION:        return "MIME-Version";
    case field_name::REFERER:             return "Referer";
    case field_name::RETRY_AFTER:         return "Retry-After";
    case field_name::SERVER:              return "Server";
    case field_name::USER_AGENT:          return "User-Agent";
    case field_name::VARY:                return "Vary";
    
    case field_name::ETAG:                return "ETag";
    case field_name::IF_MATCH:            return "If-Match";
    case field_name::IF_MODIFIED_SINCE:   return "If-Modified-Since";
    case field_name::IF_NONE_MATCH:       return "If-None-Match";
    case field_name::IF_UNMODIFIED_SINCE: return "If-Unmodified-Since";
    case field_name::LAST_MODIFIED:       return "Last-Modified";
    
    case field_name::ACCEPT_RANGES:       return "Accept-Ranges";
    case field_name::CONTENT_RANGE:       return "Content-Range";
    case field_name::IF_RANGE:            return "If-Range";
    case field_name::RANGE:               return "Range";
    
    case field_name::AGE:                 return "Age";
    case field_name::CACHE_CONTROL:       return "Cache-Control";
    case field_name::EXPIRES:             return "Expires";
    case field_name::PRAGMA:              return "Pragma";
    case field_name::WARNING:             return "Warning";
    
    case field_name::AUTHORIZATION:       return "Authorization";
    case field_name::PROXY_AUTHENTICATE:  return "Proxy-Authenticate";
    case field_name::PROXY_AUTHORIZATION: return "Proxy-Authorization";
    case field_name::WWW_AUTHENTICATE:    return "WWW-Authenticate";
    
    default: return "";
    }
}


std::string
http::status_code_string(status_code __id) noexcept
{
    switch (__id) {
    case status_code::CONTINUE:                        return "Continue";
    case status_code::SWITCHING_PROTOCOLS:             return "Switching Protocols";
    case status_code::PROCESSING:                      return "Processing";
    case status_code::EARLY_HINTS:                     return "Early Hints";
    case status_code::OK:                              return "OK";
    case status_code::CREATED:                         return "Created";
    case status_code::ACCEPTED:                        return "Accepted";
    case status_code::NON_AUTHORITATIVE_INFO:          return "Non-Authoritative Info";
    case status_code::NO_CONTENT:                      return "No Content";
    case status_code::RESET_CONTENT:                   return "Reset Content";
    case status_code::PARTIAL_CONTENT:                 return "Partial Content";
    case status_code::MULTI_STATUS:                    return "Multi-Status";
    case status_code::ALREADY_REPORTED:                return "Already Reported";
    case status_code::IM_USED:                         return "IM Used";
    case status_code::MULTIPLE_CHOICES:                return "Multiple choices";
    case status_code::MOVED_PERMANENTLY:               return "Moved Permanently";
    case status_code::FOUND:                           return "Found";
    case status_code::SEE_OTHER:                       return "See Other";
    case status_code::NOT_MODIFIED:                    return "Not Modified";
    case status_code::USE_PROXY:                       return "Use Proxy";
    case status_code::UNUSED:                          return "(unused)";
    case status_code::TEMPORARY_REDIRECT:              return "Temporary Redirect";
    case status_code::PERMANENT_REDIRECT:              return "Permanent Redirect";
    case status_code::BAD_REQUEST:                     return "Bad Request";
    case status_code::UNAUTHORIZED:                    return "Unauthorized";
    case status_code::PAYMENT_REQUIRED:                return "Payment Required";
    case status_code::FORBIDDEN:                       return "Forbidden";
    case status_code::NOT_FOUND:                       return "Not Found";
    case status_code::METHOD_NOT_ALLOWED:              return "Method Not Allowed";
    case status_code::NOT_ACCEPTABLE:                  return "Not Acceptable";
    case status_code::PROXY_AUTHENTICATION_REQUIRED:   return "Proxy Authentication Required";
    case status_code::REQUEST_TIMEOUT:                 return "Request Timeout";
    case status_code::CONFLICT:                        return "Conflict";
    case status_code::GONE:                            return "Gone";
    case status_code::LENGTH_REQUIRED:                 return "Length Required";
    case status_code::PRECONDITION_FAILED:             return "Precondition Failed";
    case status_code::PAYLOAD_TOO_LARGE:               return "Payload Too Large";
    case status_code::URI_TOO_LONG:                    return "URI Too Long";
    case status_code::UNSUPPORTED_MEDIA_TYPE:          return "Unsupported Media Type";
    case status_code::RANGE_NOT_SATISFIABLE:           return "Range Not Satisfiable";
    case status_code::EXPECTATION_FAILED:              return "Expectation Failed";
    case status_code::MISDIRECTED_REQUEST:             return "Misdirected Request";
    case status_code::UNPROCESSABLE_ENTITY:            return "Unprocessable Entity";
    case status_code::LOCKED:                          return "Locked";
    case status_code::FAILED_DEPENDENCY:               return "Failed Dependency";
    case status_code::TOO_EARLY:                       return "Too Early";
    case status_code::UPGRADE_REQUIRED:                return "Upgrade Required";
    case status_code::PRECONDITION_REQUIRED:           return "Precondition Required";
    case status_code::TOO_MANY_REQUESTS:               return "Too Many Request";
    case status_code::REQUEST_HEADER_FIELDS_TOO_LARGE: return "Request Header Fields Too Large";
    case status_code::UNAVAILABLE_FOR_LEGAL_REASONS:   return "Unavailable For Legal Reasons";
    case status_code::INTERNAL_SERVER_ERROR:           return "Internal Server Error";
    case status_code::NOT_IMPLEMENTED:                 return "Not Implemented";
    case status_code::BAD_GATEWAY:                     return "Bad Gateway";
    case status_code::SERVICE_UNAVAILABLE:             return "Service Unavailable";
    case status_code::GATEWAY_TIMEOUT:                 return "Gateway Timeout";
    case status_code::HTTP_VERSION_NOT_SUPPORTED:      return "HTTP Version Not Supported";
    case status_code::VARIANT_ALSO_NEGOTIATES:         return "Variant Also Negotiates";
    case status_code::INSUFFICIENT_STORAGE:            return "Insufficient Storage";
    case status_code::LOOP_DETECTED:                   return "Loop Detected";
    case status_code::NOT_EXTENDED:                    return "Not Extended";
    case status_code::NETWORK_AUTHENTICATION_REQUIRED: return "Network Authentication Required";
    default: return "";
    }
}
