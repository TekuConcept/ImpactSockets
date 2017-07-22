/**
 * Created by TekuConcept on July 21, 2017
 */

#include "RFC6455.h"
#include <sstream>

using namespace Impact;

void RFC6455::URI::DerefFragment(std::string &uri) {
    std::ostringstream ss;
    const char* buffer = uri.c_str();
    for(unsigned int i = 0; i < uri.length(); i++) {
        if(buffer[i] == '#') break;
        else                 ss << buffer[i];
    }
    uri.assign(ss.str());
}

void RFC6455::URI::EscapeAllPound(std::string &uri) {
    std::ostringstream ss;
    const char* buffer = uri.c_str();
    for(unsigned int i = 0; i < uri.length(); i++) {
        if(buffer[i] == '#') ss << "%%23";
        else                 ss << buffer[i];
    }
    uri.assign(ss.str());
}

std::string RFC6455::URI::getProtocol() {
    return "ws";
}

std::string RFC6455::URI::getSecureProtocol() {
    return "wss";
}

bool RFC6455::URI::isValid(const Info *info) {
    (void)info;
    return true;
}

