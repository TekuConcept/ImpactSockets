/**
 * Created by TekuConcept on July 21, 2017
 */

#include "RFC2616.h"
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

unsigned int RFC6455::URI::getDefaultPort() {
    return 80;
}

unsigned int RFC6455::URI::getDefaultSecurePort() {
    return 443;
}

bool RFC6455::URI::parseURI(std::string uri, Info &info) {
    // smallest uri name length: 8
    // ex: ws://a.z
    if(uri.length() < 8) return false;
    const int MAX_HOST = 254; // 253 + '.'
    const int MAX_LABEL = 63;
    const char* buffer = uri.c_str();
    int length = uri.length();
    
    // validate protocol: "ws:" or "wss:"
    std::string scheme;
    if(!RFC2616::URI::parseScheme(uri, scheme)) return false;
    if(scheme == "ws") info.secure = false;
    else if(scheme == "wss") info.secure = true;
    else return false;
    int idx = scheme.length() + 1;
    
    if(buffer[idx] != '/' || buffer[idx+1] != '/')
        return false;
    idx += 1; // 2nd increment in loop below

    // validate host name: [a-zA-Z0-9-.]
    std::ostringstream host;
    int labelLen = 0, hostLen = 0;
    bool hasPort = false, firstChar = true;
    do {
        idx++;
        if(firstChar) {
            // RFC 952: non-alphanumeric chars as the first char
            // are not allowed.
            if(buffer[idx] == '-') return false;
            else firstChar = false;
        }
        if(
            (buffer[idx] >= 'a' && buffer[idx] <= 'z') || // a-z
            (buffer[idx] >= 'A' && buffer[idx] <= 'Z') || // A-Z
            (buffer[idx] >= '0' && buffer[idx] <= '9') ||
            (buffer[idx] == '-')
        ) {
            host << buffer[idx];
            labelLen++;
            hostLen++;
        }
        else if(buffer[idx] == '.') {
            // end of label
            host << buffer[idx];
            labelLen = 0;
            hostLen++;
        }
        else if(buffer[idx] == ':') {
            // doesn't account for IPv6 addresses
            // :- TODO -: fix this for complience
            hasPort = true;
            idx--; // reverse increment (canceled out below)
            break;
        }
        else if(buffer[idx] == '/')
            break;
        else return false;
        if(labelLen > MAX_LABEL || hostLen > MAX_HOST) return false;
    } while((idx+1) < length);
    info.host = host.str();
    idx++;
    
    // validate port: [0-9]
    if(hasPort) {
        if((length - idx) > 1) {
            std::ostringstream port;
            const int MAX_PORT_LEN = 5; // 65535
            int portLen = 0;
            do {
                idx++; // ignore ':' and then increment there after
                if(buffer[idx] >= '0' && buffer[idx] <= '9') {
                    port << buffer[idx];
                    portLen++;
                }
                else if(buffer[idx] == '/') break;
                else return false;
                if(portLen > MAX_PORT_LEN) {
                    return false;
                }
            } while((idx+1) < length);
            info.port = std::stoi(port.str());
            if(info.port > 65535) return false;
        }
        else { // ws://host:/ is legal; use default port
            if(info.secure) info.port = RFC6455::URI::getDefaultSecurePort();
            else            info.port = RFC6455::URI::getDefaultPort();
        }
    }
    else {
        if(info.secure) info.port = RFC6455::URI::getDefaultSecurePort();
        else            info.port = RFC6455::URI::getDefaultPort();
    }

    if((length - idx) >= 1) {
        std::ostringstream resource;
        while(idx < length) {
            if(buffer[idx] == '#') break;
            resource << buffer[idx];
            idx++;
        }
        info.resourceName = resource.str();
    }
    
    return true;
}

bool RFC6455::URI::validate(std::string uri) {
    Info info;
    return parseURI(uri, info);
}

std::string RFC6455::getRequestHeader(URI::Info info) {
    std::ostringstream os;
    os << RFC2616::Request::getRequestLine(
        RFC2616::Request::METHOD::GET,
        info.resourceName
    );
    return os.str();
}