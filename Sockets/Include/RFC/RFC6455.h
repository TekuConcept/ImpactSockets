/**
 * Created by TekuConcept on July 21, 2017
 */

#ifndef RFC6455_H
#define RFC6455_H

#include <string>
#include "RFC/2616"

namespace Impact {
    namespace RFC6455 {
	    enum State {
    	    CONNECTING
        };
	
        namespace URI {
            void DerefFragment(std::string &uri);
            void EscapeAllPound(std::string &uri);
        }
        
        std::string getRequestMessage(RFC2616::URI info);
    }
}

#endif