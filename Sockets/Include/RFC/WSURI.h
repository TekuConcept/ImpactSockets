/**
 * Created by TekuConcept on July 25, 2017
 */

#ifndef RFC_WSURI_H
#define RFC_WSURI_H

#include <RFC/URI.h>

namespace Impact{
namespace RFC6455 {
    class WSURI : public RFC2616::URI {
    public:
        WSURI(std::string uri);
        WSURI(RFC2616::URI uri);
        
        bool secure();
        
        static bool validate(std::string uri);
    
    private:
        bool init();
    };
}}

#endif