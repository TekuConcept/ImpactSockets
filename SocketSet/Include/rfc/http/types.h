/**
 * Created by TekuConcept on December 15, 2018
 */

#ifndef IMPACT_HTTP_TYPES_H
#define IMPACT_HTTP_TYPES_H

#include <string>

namespace impact {
namespace http {
    typedef enum class method {
        /* - RFC 7231 - */
        CONNECT, // Section 4.3.6
        DELETE,  // Section 4.3.5
        GET,     // Section 4.3.1
        HEAD,    // Section 4.3.2
        OPTIONS, // Section 4.3.7
        POST,    // Section 4.3.3
        PUT,     // Section 4.3.4
        TRACE,   // Section 4.3.8
    } Method;
    
    typedef enum class field_name {
        /* - RFC 6265 - */
        SET_COOKIE,          // Section 5.2

        /* - RFC 7230 - */
        CONNECTION,          // Section 6.1
        CONTENT_LENGTH,      // Section 3.3.2
        HOST,                // Section 5.4
        TE,                  // Section 4.3
        TRAILER,             // Section 4.4
        TRANSFER_ENCODING,   // Section 3.3.1
        UPGRADE,             // Section 6.7
        VIA,                 // Section 5.7.1
        CLOSE,               // Section 8.1 (reserved)

        /* - RFC 7231 - */
        ACCEPT,              // Section 5.3.2
        ACCEPT_CHARSET,      // Section 5.3.3
        ACCEPT_ENCODING,     // Section 5.3.4
        ACCEPT_LANGUAGE,     // Section 5.3.5
        ALLOW,               // Section 7.4.1
        CONTENT_ENCODING,    // Section 3.1.2.2
        CONTENT_LANGUAGE,    // Section 3.1.3.2
        CONTENT_LOCATION,    // Section 3.1.4.2
        CONTENT_TYPE,        // Section 3.1.1.5
        DATE,                // Section 7.1.1.2
        EXPECT,              // Section 5.1.1
        FROM,                // Section 5.5.1
        LOCATION,            // Section 7.1.2
        MAX_FORWARDS,        // Section 5.1.2
        MIME_VERSION,        // Appendix A.1
        REFERER,             // Section 5.5.2
        RETRY_AFTER,         // Section 7.1.3
        SERVER,              // Section 7.4.2
        USER_AGENT,          // Section 5.5.3
        VARY,                // Section 7.1.4
        
        /* - RFC 7232 - */
        ETAG,                // Section 2.3
        IF_MATCH,            // Section 3.1
        IF_MODIFIED_SINCE,   // Section 3.3
        IF_NONE_MATCH,       // Section 3.2
        IF_UNMODIFIED_SINCE, // Section 3.4
        LAST_MODIFIED,       // Section 2.2
        
        /* - RFC 7233 - */
        ACCEPT_RANGES,       // Section 2.3
        CONTENT_RANGE,       // Section 4.2
        IF_RANGE,            // Section 3.2
        RANGE,               // Section 3.1
        
        /* - RFC 7234 - */
        AGE,                 // Section 5.1
        CACHE_CONTROL,       // Section 5.2
        EXPIRES,             // Section 5.3
        PRAGMA,              // Section 5.4
        WARNING,             // Section 5.5
        
        /* - RFC 7235 - */
        AUTHORIZATION,       // Section 4.2
        PROXY_AUTHENTICATE,  // Section 4.3
        PROXY_AUTHORIZATION, // Section 4.4
        WWW_AUTHENTICATE,    // Section 4.1
    } FieldName;
    
    /* RFC 7231                                          */
    /* http://www.iana.org/assignments/http-status-codes */

    typedef enum class status_code {
        CONTINUE                        = 100, // Section 6.2.1
        SWITCHING_PROTOCOLS             = 101, // Section 6.2.2
        PROCESSING                      = 102, // [RFC2518]
        EARLY_HINTS                     = 103, // [RFC8297]
        OK                              = 200, // Section 6.3.1
        CREATED                         = 201, // Section 6.3.2
        ACCEPTED                        = 202, // Section 6.3.3
        NON_AUTHORITATIVE_INFO          = 203, // Section 6.3.4
        NO_CONTENT                      = 204, // Section 6.3.5
        RESET_CONTENT                   = 205, // Section 6.3.6
        PARTIAL_CONTENT                 = 206, // Section 4.1 of [RFC7233]
        MULTI_STATUS                    = 207, // [RFC4918]
        ALREADY_REPORTED                = 208, // [RFC5842]
        IM_USED                         = 226, // [RFC3229]
        MULTIPLE_CHOICES                = 300, // Section 6.4.1
        MOVED_PERMANENTLY               = 301, // Section 6.4.2
        FOUND                           = 302, // Section 6.4.3
        SEE_OTHER                       = 303, // Section 6.4.4
        NOT_MODIFIED                    = 304, // Section 4.1 of [RFC7232]
        USE_PROXY                       = 305, // Section 6.4.5
        UNUSED                          = 306, // [RFC7231, Section 6.4.6]
        TEMPORARY_REDIRECT              = 307, // Section 6.4.7
        PERMANENT_REDIRECT              = 308, // [RFC7538]
        BAD_REQUEST                     = 400, // Section 6.5.1
        UNAUTHORIZED                    = 401, // Section 3.1 of [RFC7235]
        PAYMENT_REQUIRED                = 402, // Section 6.5.2
        FORBIDDEN                       = 403, // Section 6.5.3
        NOT_FOUND                       = 404, // Section 6.5.4
        METHOD_NOT_ALLOWED              = 405, // Section 6.5.5
        NOT_ACCEPTABLE                  = 406, // Section 6.5.6
        PROXY_AUTHENTICATION_REQUIRED   = 407, // Section 3.2 of [RFC7235]
        REQUEST_TIMEOUT                 = 408, // Section 6.5.7
        CONFLICT                        = 409, // Section 6.5.8
        GONE                            = 410, // Section 6.5.9
        LENGTH_REQUIRED                 = 411, // Section 6.5.10
        PRECONDITION_FAILED             = 412, // Section 4.2 of [RFC7232]
        PAYLOAD_TOO_LARGE               = 413, // Section 6.5.11
        URI_TOO_LONG                    = 414, // Section 6.5.12
        UNSUPPORTED_MEDIA_TYPE          = 415, // Section 6.5.13
        RANGE_NOT_SATISFIABLE           = 416, // Section 4.4 of [RFC7233]
        EXPECTATION_FAILED              = 417, // Section 6.5.14
        MISDIRECTED_REQUEST             = 421, // [RFC7540, Section 9.1.2]
        UNPROCESSABLE_ENTITY            = 422, // [RFC4918]
        LOCKED                          = 423, // [RFC4918]
        FAILED_DEPENDENCY               = 424, // [RFC4918]
        TOO_EARLY                       = 425, // [RFC-ietf-httpbis-replay-04]
        UPGRADE_REQUIRED                = 426, // Section 6.5.15
        PRECONDITION_REQUIRED           = 428, // [RFC6585]
        TOO_MANY_REQUESTS               = 429, // [RFC6585]
        REQUEST_HEADER_FIELDS_TOO_LARGE = 431, // [RFC6585]
        UNAVAILABLE_FOR_LEGAL_REASONS   = 451, // [RFC7725]
        INTERNAL_SERVER_ERROR           = 500, // Section 6.6.1
        NOT_IMPLEMENTED                 = 501, // Section 6.6.2
        BAD_GATEWAY                     = 502, // Section 6.6.3
        SERVICE_UNAVAILABLE             = 503, // Section 6.6.4
        GATEWAY_TIMEOUT                 = 504, // Section 6.6.4
        HTTP_VERSION_NOT_SUPPORTED      = 505, // Section 6.6.6
        VARIANT_ALSO_NEGOTIATES         = 506, // [RFC2295]
        INSUFFICIENT_STORAGE            = 507, // [RFC4918]
        LOOP_DETECTED                   = 508, // [RFC5842]
        NOT_EXTENDED                    = 510, // [RFC2774]
        NETWORK_AUTHENTICATION_REQUIRED = 511  // [RFC6585]
    } StatusCode;
    
    bool idempotent(method id) noexcept;
    std::string method_string(method id) noexcept;
    std::string field_name_string(field_name id) noexcept;
    std::string status_code_string(status_code id) noexcept;
}}

#endif
