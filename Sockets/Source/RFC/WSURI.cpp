/**
 * Created by TekuConcept on July 25, 2017
 */

#include "RFC/WSURI.h"
#include "RFC/Const2616.h"
#include <exception>

using namespace Impact;
using namespace RFC6455;

WSURI::WSURI(std::string uri) : URI(uri) {
    if(!init()) throw std::exception();
}

WSURI::WSURI(RFC2616::URI uri) : URI(uri) {
    if(!init()) throw std::exception();
}

bool WSURI::init() {
    if(_scheme_ != "ws" && _scheme_ != "wss") return false;
    if(_port_ == 0) {
        if(_scheme_ == "wss") _port_ = RFC2616::SECURE_PORT;
        else if(_scheme_ == "ws") _port_ = RFC2616::PORT;
    }
    return true;
}

bool WSURI::secure() {
    return _scheme_ == "wss";
}

bool WSURI::validate(std::string uri) {
    bool check = false;
    RFC2616::URI _uri_ = RFC2616::URI::tryParse(uri, check);
    
    if(check) {
        if(_uri_.scheme() == "ws" || _uri_.scheme() == "wss")
            return true;
        else return false;
    }
    else return false;
}