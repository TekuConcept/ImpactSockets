/**
 * Created by TekuConcept on July 25, 2017
 */

#ifndef RFC_URI_H
#define RFC_URI_H

#include <string>

namespace Impact {
namespace RFC2616 {
    class URI {
    public:
        URI(std::string uri);
        std::string scheme();
        std::string host();
        std::string resource();
        unsigned int port();
        
        static bool validate(std::string uri);
        static URI trial(std::string uri, bool &success);

    protected:
        std::string _scheme_, _host_, _resource_;
        unsigned int _port_;
        
        bool parse(std::string uri);
        bool parseScheme(std::string uri);
        bool parseIPv6Host(std::string uri, unsigned int &offset);
        bool parseHost(std::string uri, unsigned int &offset);
        bool parsePort(std::string uri, unsigned int &offset);
    
    private:
        URI();
    };
}}

#endif