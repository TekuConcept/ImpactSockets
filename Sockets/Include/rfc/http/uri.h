/**
 * Created by TekuConcept on August 9, 2018
 */

#ifndef _IMPACT_HTTP_URI_H_
#define _IMPACT_HTTP_URI_H_

#include <string>

#include "rfc/uri.h"

namespace impact {
namespace http {
    /**
     * NOTE: This object is simply a temporary placeholder
     * to enforce protocol requirements. In time this object
     * may be completely omitted in favor of impact::uri to
     * make the API more convinient (less stuff to worry about).
     * This object may be considered more or less a reference
     * for the time being.
     */
    class uri : public impact::uri {
    public:
        uri();
        uri(const impact::uri& r);
        virtual ~uri();
        
        static bool parse(std::string value, impact::uri* result);
        static bool parse(std::string value, impact::uri* result,
            struct uri_parser_opts /* hidden */);
    
    private:
        static void _S_init(struct uri_parser_opts*);
    };
}}

#endif