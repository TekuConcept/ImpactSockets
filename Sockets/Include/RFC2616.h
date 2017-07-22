/**
 * Created by TekuConcept on July 22, 2017
 */

#ifndef RFC2616_H
#define RFC2616_H

#include <string>
#include <vector>

namespace Impact {
    namespace RFC2616 {
        const std::string HTTP_VERSION = "HTTP/1.1";
        const std::string CRLF = "\r\n";
        const char SP = ' ';

        // RFC 2616 Section 10: Status Code Definitions
        typedef enum STATUS {
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
        std::string getStatusString(STATUS code);
        
        namespace URI {
            typedef struct Info {
                std::string scheme;
                std::string host;
                unsigned int port;
                std::string resourceName;
            } Info;
            
            bool parseScheme(std::string uri, std::string &scheme);
            bool parse(std::string uri, Info &info);
            bool validate(std::string uri);
        }

        namespace Request {
            typedef enum METHOD {
                OPTIONS = 0,
                GET,
                HEAD,
                POST,
                PUT,
                DELETE,
                TRACE,
                CONNECT
            } METHOD;

            typedef struct Info {
                METHOD method;
                std::string requestURI;
                std::string version;
                std::vector<std::string> headers;
                std::string body;
            } Info;

            std::string getMethodString(METHOD code);

            std::string getRequestLine(METHOD code, std::string reqURI);
            
            bool parseRequest(std::string request, Info &info);

            bool validate(std::string request);
        }
    }
}

#endif