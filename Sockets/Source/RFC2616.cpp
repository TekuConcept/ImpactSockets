/**
 * Created by TekuConcept on July 22, 2017
 */

#include "RFC2616.h"
#include <map>
#include <sstream>

#include <iostream>
#define DMSG(x) std::cerr << x << std::endl

using namespace Impact;

typedef std::pair<int, std::string> Token;

std::map <int, std::string> STATUS_NAMES = {
    // -- informational --
    Token(100, "CONTINUE"),
    Token(101, "SWITCHING"),
    
    // -- successful --
    Token(200, "OK"),
    Token(201, "CREATED"),
    Token(202, "ACCEPTED"),
    Token(203, "NON AUTHORITATIVE"),
    Token(204, "NO CONTENT"),
    Token(205, "RESET CONTENT"),
    Token(206, "PARTIAL CONTENT"),
    
    // -- redirection --
    Token(300, "MULTIPLE CHOICES"),
    Token(301, "MOVED PERMANENTLY"),
    Token(302, "FOUND"),
    Token(303, "SEE OTHER"),
    Token(304, "NOT MODIFIED"),
    Token(305, "USE PROXY"),
    // 306 (unused)
    Token(307, "TEMPORARY REDIRECT"),
    
    // -- client error --
    Token(400, "BAD REQUEST"),
    Token(401, "UNAUTHORIZED"),
    Token(402, "PAYMENT REQUIRED"),
    Token(403, "FORBIDDEN"),
    Token(404, "NOT FOUND"),
    Token(405, "METHOD NOT ALLOWED"),
    Token(406, "NOT ACCEPTABLE"),
    Token(407, "PROXY AUTHENTICATION REQUIRED"),
    Token(408, "REQUEST TIMEOUT"),
    Token(409, "CONFLICT"),
    Token(410, "GONE"),
    Token(411, "LENGTH REQUIRED"),
    Token(412, "PRECONDITION FAILED"),
    Token(413, "REQUEST ENTITY TOO LARGE"),
    Token(414, "REQUEST URI TOO LONG"),
    Token(415, "UNSUPPORTED MEDIA TYPE"),
    Token(416, "REQUEST RANGE NOT SATISFIABLE"),
    Token(417, "EXPECTATION FAILED"),
    
    // -- server error --
    Token(500, "INTERNAL SERVER ERROR"),
    Token(501, "NOT IMPLEMENTED"),
    Token(502, "BAD GATEWAY"),
    Token(503, "SERVICE UNAVAILBLE"),
    Token(504, "GATEWAY TIMEOUT"),
    Token(505, "HTTP VERSION NOT SUPPORTED"),
};

std::map <int, std::string> METHOD_NAMES = {
    Token(0, "OPTIONS"),
    Token(1, "GET"),
    Token(2, "HEAD"),
    Token(3, "POST"),
    Token(4, "PUT"),
    Token(5, "DELETE"),
    Token(6, "TRACE"),
    Token(7, "CONNECT")
};

std::string RFC2616::getStatusString(STATUS code) {
    return STATUS_NAMES[(int)code];
}

bool RFC2616::URI::parseScheme(std::string uri, std::string &scheme) {
    std::ostringstream os;
    const char* buffer = uri.c_str();
    unsigned int idx = 0, length = uri.length();
    bool foundDelimiter = false;
    while(idx < length) {
        if(buffer[idx] == ':') {
            foundDelimiter = true;
            break;
        }
        os << buffer[idx];
        idx++;
    }
    scheme.assign(os.str());
    return foundDelimiter;
}

bool parseHostHelperIPv6(const char* buffer, unsigned int length,
    unsigned int &idx, std::ostringstream &os) {
    int colonDelimiterCount = 0;

    int labelLen = 0, hostLen = 0;
    while(idx < length) {
        if(buffer[idx] == '/') return false; // didn't close with ']'
        else if(buffer[idx] == ':') {
            labelLen = 0;
            hostLen++;
            os << buffer[idx];
            
            colonDelimiterCount++;
            // IPv6 doesn't have more than 8 groups
            if (colonDelimiterCount > 7) return false;
        }
        else if(buffer[idx] == ']') {
            idx++; // align with parseHostHelper() function's return index
            // expecting "::" minimum
            if(colonDelimiterCount < 2) return false;
            else return true;
        }
        else if( // only allow legal hex values
            (buffer[idx] >= 'a' && buffer[idx] <= 'f') || // a-z
            (buffer[idx] >= 'A' && buffer[idx] <= 'F') || // A-Z
            (buffer[idx] >= '0' && buffer[idx] <= '9')
        ) {
            labelLen++;
            hostLen++;
            os << buffer[idx];
        }
        else return false;
        
        // labels are only 4 hex chars long
        // a fully expanded IPv6 address (including all ':') is at most 39 chars
        if(labelLen > 4 || hostLen > 39) return false;
        
        idx++;
    }
    
    return false;
}

bool parseHostHelper(const char* buffer, unsigned int length,
    unsigned int &idx, std::ostringstream &os) {
    while(idx < length) {
        if(buffer[idx] == '/') break;
        else if(buffer[idx] == ':') break;
        // only allow characters that match [a-zA-Z0-9.-]
        os << buffer[idx];
    }
    return true;
}

bool RFC2616::URI::parseHost(std::string uri, std::string &host,
    unsigned int &port) {
    std::string scheme;
    if(!parseScheme(uri, scheme)) return false;
    
    const char *buffer = uri.c_str();
    unsigned int length = uri.length();
    if(length < 7) return false; // smallest uri: "a://b.c"
    
    std::ostringstream os;
    unsigned int idx = scheme.length() + 3; // skip "://"
    
    if(buffer[idx] == '[') { // IPv6 enclosed: "[::]"
        idx++;
        if((length - idx) < 3) return false; // allow for at least [::]
        if(!parseHostHelperIPv6(buffer, length, idx, os)) return false;
    }
    else {
        if(!parseHostHelper(buffer, length, idx, os)) return false;
    }
    host = os.str();
    
    // done getting the host
    // now idx should either be pointing at ':' for port,
    // '/' for resource name, or end of uri
    port = 0;
    
    return true;
}

bool RFC2616::URI::validateHost(std::string uri) {
    std::string host;
    unsigned int port;
    return parseHost(uri, host, port);
}

std::string RFC2616::Request::getMethodString(METHOD code) {
    return METHOD_NAMES[(int)code];
}

std::string RFC2616::Request::getRequestLine(METHOD code, std::string reqURI) {
    // validate:
    // request URI must either be a wild card "*", start with '/',
    // or be a fully defined URI path.
    if(reqURI.length() == 0) throw -1;
    else if(reqURI.at(0) == '*' && reqURI.length() != 1)
        throw -2;
    else if(reqURI.at(0) != '/' && reqURI.at(0) != '*' &&
        (reqURI.find("http") != 0))
        throw -3;

    // compose
    std::ostringstream os;
    os << getMethodString(code) << SP << reqURI << SP << HTTP_VERSION << CRLF;
    return os.str();
}

bool parseRequestMethod(std::string request, RFC2616::Request::Info &info) {
    switch(request.at(0)) {
        case 'C': info.method = RFC2616::Request::METHOD::CONNECT; break;
        case 'D': info.method = RFC2616::Request::METHOD::DELETE;  break;
        case 'G': info.method = RFC2616::Request::METHOD::GET;     break;
        case 'H': info.method = RFC2616::Request::METHOD::HEAD;    break;
        case 'O': info.method = RFC2616::Request::METHOD::OPTIONS; break;
        case 'P':
            if(request.at(1) == 'O')
                info.method = RFC2616::Request::METHOD::POST;
            else if(request.at(1) == 'U')
                info.method = RFC2616::Request::METHOD::PUT;
            else return false;
            break;
        case 'T': info.method = RFC2616::Request::METHOD::TRACE;   break;
        default: return false;
    }
    // double check because we only initially checked the first character(s)
    if(request.find(METHOD_NAMES[(int)info.method]) != 0) return false;
    return true;
}

bool RFC2616::Request::parseRequest(std::string request, Info &info) {
    // validate length
    
    // first word must be a method and is case-sensitive
    if(!parseRequestMethod(request, info)) return false;
    int idx = METHOD_NAMES[(int)info.method].length();
    idx++; // skip SP
    
    // todo get request URI
    //      get http version
    //      check valid line ending
    
    (void)info;
    return true;
}

bool RFC2616::Request::validate(std::string request) {
    Info info;
    // validate resource uri
    return parseRequest(request, info);
}