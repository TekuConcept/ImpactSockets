/**
 * Created by TekuConcept on July 21, 2017
 */

#include "RFC/RFC2616.h"
#include "RFC/RFC6455.h"
#include "RFC/RequestMessage.h"
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

unsigned int RFC6455::URI::getDefaultPort() {
    return 80;
}

unsigned int RFC6455::URI::getDefaultSecurePort() {
    return 443;
}

bool RFC6455::URI::parse(std::string uri, Info &info) {
    // smallest uri name length: 8
    // ex: ws://a.z
    if(uri.length() < 8) return false;
    
    RFC2616::URI::Info basicInfo;
    if(!RFC2616::URI::parse(uri, basicInfo)) return false;
    
    // validate protocol: "ws:" or "wss:"
    std::string scheme;
    if(basicInfo.scheme == "ws")       info.secure = false;
    else if(basicInfo.scheme == "wss") info.secure = true;
    else return false;
    
    info.host         = basicInfo.host;
    info.port         = basicInfo.port;
    info.resourceName = basicInfo.resourceName;
    
    if(basicInfo.port == 0)
        info.port = (info.secure ? getDefaultSecurePort() : getDefaultPort());
    
    return true;
}

bool RFC6455::URI::validate(std::string uri) {
    Info info;
    return parse(uri, info);
}

std::string RFC6455::getRequestMessage(URI::Info info) {
    RFC2616::Request::Message message(
        RFC2616::Request::METHOD::GET,
        info.resourceName
    );
    return message.toString();
}