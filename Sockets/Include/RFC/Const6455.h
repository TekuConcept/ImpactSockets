/**
 * Created by TekuConcept on July 27, 2017
 */

#ifndef RFC_CONST6455_H
#define RFC_CONST6455_H

#include <string>
#include "RFC/Const2616.h"

namespace Impact {
namespace RFC6455 {
    typedef enum STATE {
	    CONNECTING,
	    OPEN,
	    CLOSED
    } STATE;
    
    typedef enum HEADER {
        SecWebSocketKey = 0,
        SecWebSocketExtensions,
        SecWebSocketAccept,
        SecWebSocketProtocol,
        SecWebSocketVersion
    } HEADER;
    
    bool findHeader(const std::string header, HEADER &code);
    bool findHeader(const RFC2616::string header, HEADER &code);
    std::string toString(HEADER code);
}}

#endif