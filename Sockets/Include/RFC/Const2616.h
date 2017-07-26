/**
 * Created by TekuConcept on July 24, 2017
 */

#ifndef RFC_CONST2616_H
#define RFC_CONST2616_H

#include <string>
#include <iostream>
#include "RFC/String.h"

namespace Impact {
namespace RFC2616 {
    const unsigned int HTTP_MAJOR = 1;
    const unsigned int HTTP_MINOR = 1;
    const std::string HTTP_PREFIX = "HTTP/";
    const std::string CRLF = "\r\n";
    const char SP = ' ';
    const char HWS= '\t';
    const unsigned short PORT = 80;
    const unsigned short SECURE_PORT = 443;

    typedef enum STATUS {
        // RFC 2616 Section 10
        // -- informational --
        CONTINUE=100,
        SWITCHING=101,
        
        // -- successful --
        OK=200,
        CREATED=201,
        ACCEPTED=202,
        NON_AUTHORITATIVE=203,
        NO_CONTENT=204,
        RESET_CONTENT=205,
        PARTIAL_CONTENT=206,
        
        // -- redirection --
        MULTIPLE_CHOICES=300,
        MOVED_PERMANENTLY=301,
        FOUND=302,
        SEE_OTHER=303,
        NOT_MODIFIED=304,
        USE_PROXY=305,
        // 306 (unused)
        TEMP_REDIRECT=307,
        
        // -- client error --
        BAD_REQUEST=400,
        UNAUTHORIZED=401,
        PAYMENT_REQUIRED=402,
        FORBIDDEN=403,
        NOT_FOUND=404,
        METHOD_NOT_ALLOWED=405,
        NOT_ACCEPTABLE=406,
        PROXY_AUTH_REQUIRED=407,
        REQUEST_TIMEOUT=408,
        CONFLICT=409,
        GONE=410,
        LENGTH_REQUIRED=411,
        PRECONDITION_FAILED=412,
        REQUEST_ENTITY_TOO_LARGE=413,
        REQUEST_URI_TOO_LONG=414,
        UNSUPPORTED_MEDIA_TYPE=415,
        REQUEST_RANGE_NOT_SATISFIABLE=416,
        EXPECTATION_FAILED=417,
        
        // -- server error --
        INTERNAL_SERVER_ERROR=500,
        NOT_IMPLEMENTED=501,
        BAD_GATEWAY=502,
        SERVICE_UNAVAILABLE=503,
        GATEWAY_TIMEOUT=504,
        HTTP_VERSION_NOT_SUPPORTED=505
    } STATUS;
    
    typedef enum HEADER {
        // RFC 2616 Section 4.5: General Headers
        CacheControl = 0,
        Connection,
        Date,
        Pragma,
        Trailer,
        TransferEncoding,
        Upgrade,
        Via,
        Warning,
        
        // RFC 2616 Section 5.3: Request Headers
        Accept,
        AcceptCharset,
        AcceptEncoding,
        AcceptLanguage,
        Authorization,
        Expect,
        From,
        Host,
        IfMatch,
        IfModifiedSince,
        IfNoneMatch,
        IfRange,
        IfUnmodifiedSince,
        MaxForwards,
        ProxyAuthorization,
        Range,
        Referer,
        TE,
        UserAgent,
        
        // RFC 2616 Section 6.2: Response Headers
        AcceptRanges,
        Age,
        ETag,
        Location,
        ProxyAuthenticate,
        RetryAfter,
        Server,
        Vary,
        WWWAuthenticate,
        
        // RFC 2616 Section 7.1: Entity Headers
        Allow,
        ContentEncoding,
        ContentLanguage,
        ContentLength,
        ContentLocation,
        ContentMD5,
        ContentRange,
        ContentType,
        Expires,
        LastModified
    } HEADER;
    
    namespace Request {
        typedef enum METHOD {
            // RFC 2616 Section 5.1.1
            OPTIONS = 0,
            GET,
            HEAD,
            POST,
            PUT,
            DELETE,
            TRACE,
            CONNECT
        } METHOD;
    }
    
    char toLower(const char c);
    char toUpper(const char c);
    bool isWhiteSpace(const char c);
    bool validStatusCode(unsigned int code);
    bool findHeader(const std::string header, HEADER &code);
    bool findHeader(const RFC2616::string header, HEADER &code);
    
    unsigned int getHeaderKey(std::string value);
    
    std::string toString(STATUS code);
    std::string toString(Request::METHOD code);
    std::string toString(HEADER code);

    void print(std::ostream &stream, STATUS code);
    void print(std::ostream &stream, Request::METHOD code);
    void print(std::ostream &stream, HEADER code);
}}

#endif