/**
 * Created by TekuConcept on July 22, 2017
 */

#ifndef RFC2616_H
#define RFC2616_H

#include <string>
#include <vector>
#include "RFC/RFC2616Const.h"

namespace Impact {
    namespace RFC2616 {
        namespace URI {
            typedef struct Info {
                std::string scheme;
                std::string host;
                unsigned int port;
                std::string resourceName;
            } Info;
            
            bool parseScheme(std::string uri, std::string &scheme);
            bool parse(std::string uri, Info &info);
            bool validate(std::string uri);
        }

        namespace Request {
            typedef struct Info {
                METHOD method;
                std::string requestURI;
                unsigned int major;
                unsigned int minor;
                std::vector<std::string> headers;
                std::string body;
            } Info;
            
            bool parse(std::istream &request, Info &info);

            bool validate(std::string request);
        }
    }
}

#endif