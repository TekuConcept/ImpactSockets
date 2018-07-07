/**
 * Created by TekuConcept on July 25, 2017
 */

#ifndef _RFC_URI_H_
#define _RFC_URI_H_

#include <string>
#include <map>

namespace Impact {
    class URI {
    public:
        URI(std::string uri);
        
        std::string scheme();
        std::string host();
        std::string resource();
        unsigned short port();
        virtual bool secure();
        
        static bool validate(std::string uri);
        static URI tryParse(std::string uri, bool &success);
        
        static void registerScheme(std::string name, unsigned short port,
            bool secure=false);

    protected:
        std::string _scheme_, _host_, _resource_;
        unsigned short _port_;
        bool _secure_;
    
    private:
        static std::map<std::string, std::pair<bool,unsigned short>>
            _schemeMetaData_;
    
        URI();
        bool parse(std::string uri);
        bool parseScheme(std::string uri);
        void setMetaInfo();
        bool parseIPv6Host(std::string uri, unsigned int &offset);
        bool parseHost(std::string uri, unsigned int &offset);
        bool parsePort(std::string uri, unsigned int &offset);
    };
}

#endif