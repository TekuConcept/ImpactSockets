/**
 * Created by TekuConcept on July 21, 2017
 */

#ifndef RFC6455_H
#define RFC6455_H

#include <string>

namespace Impact {
    namespace RFC6455 {
	    enum State {
    	    CONNECTING
        };
	
        namespace URI {
            void DerefFragment(std::string &uri);
            void EscapeAllPound(std::string &uri);
            std::string getProtocol();
            std::string getSecureProtocol();
            
            typedef struct Info {
                std::string host;
                unsigned int port;
                std::string resourceName;
                bool secure;
                //protocols;
                //extensions;
            } Info;
            
            bool isValid(const Info *info);
        }
    }
}

#endif