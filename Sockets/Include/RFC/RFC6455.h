/**
 * Created by TekuConcept on July 21, 2017
 */

#ifndef RFC6455_H
#define RFC6455_H

#include <string>
#include "RFC/2616"
#include "RFC/WSURI.h"

namespace Impact {
    namespace RFC6455 {
	    enum State {
    	    CONNECTING
        };
        
        std::string getRequestMessage(WSURI info);
    }
}

#endif