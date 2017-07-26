/**
 * Created by TekuConcept on July 24, 2017
 */

#include <map>
#include "RFC/Const2616.h"

using namespace Impact;

typedef std::pair<unsigned int, std::string> Token;
typedef std::pair<RFC2616::string, unsigned int> Token2;

const int C_DIFF = (int)'A' - (int)'a';

std::map <unsigned int, std::string> STATUS_NAMES = {
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

std::map <unsigned int, std::string> METHOD_NAMES = {
    Token(0, "OPTIONS"),
    Token(1, "GET"),
    Token(2, "HEAD"),
    Token(3, "POST"),
    Token(4, "PUT"),
    Token(5, "DELETE"),
    Token(6, "TRACE"),
    Token(7, "CONNECT")
};

std::map <unsigned int, std::string> HEADER_NAMES = {
    // RFC 2616 Section 4.5: General Headers
    Token( 0, "Cache-Control"),
    Token( 1, "Connection"),
    Token( 2, "Date"),
    Token( 3, "Pragma"),
    Token( 4, "Trailer"),
    Token( 5, "Transfer-Encoding"),
    Token( 6, "Upgrade"),
    Token( 7, "Via"),
    Token( 8, "Warning"),
    
    // RFC 2616 Section 5.3: Request Headers
    Token( 9, "Accept"),
    Token(10, "Accept-Charset"),
    Token(11, "Accept-Encoding"),
    Token(12, "Accept-Language"),
    Token(13, "Authorization"),
    Token(14, "Expect"),
    Token(15, "From"),
    Token(16, "Host"),
    Token(17, "If-Match"),
    Token(18, "If-Modified-Since"),
    Token(19, "If-None-Match"),
    Token(20, "If-Range"),
    Token(21, "If-Unmodified-Since"),
    Token(22, "Max-Forwards"),
    Token(23, "Proxy-Authorization"),
    Token(24, "Range"),
    Token(25, "Referer"),
    Token(26, "TE"),
    Token(27, "User-Agent"),
    
    // RFC 2616 Section 6.2: Response Headers
    Token(28, "Accept-Ranges"),
    Token(29, "Age"),
    Token(30, "ETag"),
    Token(31, "Location"),
    Token(32, "Proxy-Authenticate"),
    Token(33, "Retry-After"),
    Token(34, "Server"),
    Token(35, "Vary"),
    Token(36, "WWW-Authenticate"),
    
    // RFC 2616 Section 7.1: Entity Headers
    Token(37, "Allow"),
    Token(38, "Content-Encoding"),
    Token(39, "Content-Language"),
    Token(40, "Content-Length"),
    Token(41, "Content-Location"),
    Token(42, "Content-MD5"),
    Token(43, "Content-Range"),
    Token(44, "Content-Type"),
    Token(45, "Expires"),
    Token(46, "Last-Modified"),
};

std::map <RFC2616::string, unsigned int> HEADER_NAMES_REVERSE_LOOKUP = {
    Token2("Accept", 9),
    Token2("Accept-Charset", 10),
    Token2("Accept-Encoding", 11),
    Token2("Accept-Language", 12),
    Token2("Accept-Ranges", 28),
    Token2("Authorization", 13),
    Token2("Age", 29),
    Token2("Allow", 37),
    Token2("Cache-Control", 0),
    Token2("Connection", 1),
    Token2("Content-Encoding", 38),
    Token2("Content-Language", 39),
    Token2("Content-Length", 40),
    Token2("Content-Location", 41),
    Token2("Content-MD5", 42),
    Token2("Content-Range", 43),
    Token2("Content-Type", 44),
    Token2("Date", 2),
    Token2("ETag", 30),
    Token2("Expect", 14),
    Token2("Expires", 45),
    Token2("From", 15),
    Token2("Host", 16),
    Token2("If-Match", 17),
    Token2("If-Modified-Since", 18),
    Token2("If-None-Match", 19),
    Token2("If-Range", 20),
    Token2("If-Unmodified-Since", 21),
    Token2("Last-Modified", 46),
    Token2("Location", 31),
    Token2("Max-Forwards", 22),
    Token2("Pragma", 3),
    Token2("Proxy-Authenticate", 32),
    Token2("Proxy-Authorization", 23),
    Token2("Range", 24),
    Token2("Referer", 25),
    Token2("Retry-After", 33),
    Token2("Server", 34),
    Token2("TE", 26),
    Token2("Trailer", 4),
    Token2("Transfer-Encoding", 5),
    Token2("Upgrade", 6),
    Token2("User-Agent", 27),
    Token2("Vary", 35),
    Token2("Via", 7),
    Token2("Warning", 8),
    Token2("WWW-Authenticate", 36),
};

char RFC2616::toLower(const char c) {
    if(c >= 'A' && c <='Z') {
        return (char)(c - C_DIFF);
    }
    return c;
}

char RFC2616::toUpper(const char c) {
    if(c >= 'a' && c <='z') {
        return (char)(c + C_DIFF);
    }
    return c;
}

bool RFC2616::isWhiteSpace(const char c) {
    return (c == SP) || (c == HWS);
}

bool RFC2616::validStatusCode(unsigned int code) {
    return !(STATUS_NAMES.find(code) == STATUS_NAMES.end());
}

bool RFC2616::findHeader(const std::string header, HEADER &code) {
    RFC2616::string temp(header.c_str(), header.length());
    return findHeader(temp, code);
}

bool RFC2616::findHeader(const RFC2616::string header, HEADER &code) {
    auto token = HEADER_NAMES_REVERSE_LOOKUP.find(header);
    if(token != HEADER_NAMES_REVERSE_LOOKUP.end()) {
        code = (HEADER)token->second;
        return true;
    }
    return false;
}

std::string RFC2616::toString(STATUS code) {
    return STATUS_NAMES[(int)code];
}

std::string RFC2616::toString(Request::METHOD code) {
    return METHOD_NAMES[(int)code];
}

std::string RFC2616::toString(HEADER code) {
    return HEADER_NAMES[(int)code];
}

void RFC2616::print(std::ostream &stream, STATUS code) {
    stream << toString(code);
}

void RFC2616::print(std::ostream &stream, Request::METHOD code) {
    stream << toString(code);
}

void RFC2616::print(std::ostream &stream, HEADER code) {
    stream << (int)code;
}