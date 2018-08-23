/**
 * Created by TekuConcept on August 13, 2018
 */

#include "rfc/http/field_name.h"

using namespace impact;

std::string
http::field_name_str(field_name_id __id) noexcept {
    switch (__id) {
    case field_name_id::CONNECTION:          return "Connection";
    case field_name_id::CONTENT_LENGTH:      return "Content-Length";
    case field_name_id::HOST:                return "Host";
    case field_name_id::TE:                  return "TE";
    case field_name_id::TRAILER:             return "Trailer";
    case field_name_id::TRANSFER_ENCODING:   return "Transfer-Encoding";
    case field_name_id::UPGRADE:             return "Upgrade";
    case field_name_id::VIA:                 return "Via";
    case field_name_id::CLOSE:               return "Close";
    
    case field_name_id::ACCEPT:              return "Accept";
    case field_name_id::ACCEPT_CHARSET:      return "Accept-Charset";
    case field_name_id::ACCEPT_ENCODING:     return "Accept-Encoding";
    case field_name_id::ACCEPT_LANGUAGE:     return "Accept-Language";
    case field_name_id::ALLOW:               return "Allow";
    case field_name_id::CONTENT_ENCODING:    return "Content-Encoding";
    case field_name_id::CONTENT_LANGUAGE:    return "Content-Language";
    case field_name_id::CONTENT_LOCATION:    return "Content-Location";
    case field_name_id::CONTENT_TYPE:        return "Content-Type";
    case field_name_id::DATE:                return "Date";
    case field_name_id::EXPECT:              return "Expect";
    case field_name_id::FROM:                return "From";
    case field_name_id::LOCATION:            return "Location";
    case field_name_id::MAX_FORWARDS:        return "Max-Forwards";
    case field_name_id::MIME_VERSION:        return "MIME-Version";
    case field_name_id::REFERER:             return "Referer";
    case field_name_id::RETRY_AFTER:         return "Retry-After";
    case field_name_id::SERVER:              return "Server";
    case field_name_id::USER_AGENT:          return "User-Agent";
    case field_name_id::VARY:                return "Vary";
    
    case field_name_id::ETAG:                return "ETag";
    case field_name_id::IF_MATCH:            return "If-Match";
    case field_name_id::IF_MODIFIED_SINCE:   return "If-Modified-Since";
    case field_name_id::IF_NONE_MATCH:       return "If-None-Match";
    case field_name_id::IF_UNMODIFIED_SINCE: return "If-Unmodified-Since";
    case field_name_id::LAST_MODIFIED:       return "Last-Modified";
    
    case field_name_id::ACCEPT_RANGES:       return "Accept-Ranges";
    case field_name_id::CONTENT_RANGE:       return "Content-Range";
    case field_name_id::IF_RANGE:            return "If-Range";
    case field_name_id::RANGE:               return "Range";
    
    case field_name_id::AGE:                 return "Age";
    case field_name_id::CACHE_CONTROL:       return "Cache-Control";
    case field_name_id::EXPIRES:             return "Expires";
    case field_name_id::PRAGMA:              return "Pragma";
    case field_name_id::WARNING:             return "Warning";
    
    case field_name_id::AUTHORIZATION:       return "Authorization";
    case field_name_id::PROXY_AUTHENTICATE:  return "Proxy-Authenticate";
    case field_name_id::PROXY_AUTHORIZATION: return "Proxy-Authorization";
    case field_name_id::WWW_AUTHENTICATE:    return "WWW-Authenticate";
    
    default: return "";
    }
}
