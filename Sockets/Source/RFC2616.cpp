/**
 * Created by TekuConcept on July 22, 2017
 */

#include "RFC2616.h"
#include <map>

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

std::string RFC2616::getStatusString(STATUS code) {
    return STATUS_NAMES[(int)code];
}