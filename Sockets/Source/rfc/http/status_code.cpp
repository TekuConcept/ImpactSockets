/**
 * Created by TekuConcept on August 13, 2018
 */

#include "rfc/http/status_code.h"

using namespace impact;

std::string
http::status_code_str(status_code __id) noexcept
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
