/**
 * Created by TekuConcept on August 11, 2018
 */

#ifndef _IMPACT_HTTP_STATUS_CODE_H_
#define _IMPACT_HTTP_STATUS_CODE_H_

#include <string>

namespace impact {
namespace http {

    /* RFC 7231                                          */
    /* http://www.iana.org/assignments/http-status-codes */

    enum status_code {
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
    };
    
    std::string status_code_str(status_code id) noexcept;
}}

#endif
