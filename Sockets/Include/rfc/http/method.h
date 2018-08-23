/**
 * Created by TekuConcept on August 13, 2018
 */

#ifndef _IMPACT_HTTP_METHOD_H_
#define _IMPACT_HTTP_METHOD_H_

#include <string>

namespace impact {
namespace http {
    typedef enum class method_id {
        /* - RFC 7231 - */
        CONNECT, // Section 4.3.6
        DELETE,  // Section 4.3.5
        GET,     // Section 4.3.1
        HEAD,    // Section 4.3.2
        OPTIONS, // Section 4.3.7
        POST,    // Section 4.3.3
        PUT,     // Section 4.3.4
        TRACE,   // Section 4.3.8
    } MethodID;
    
    std::string method_str(method_id id) noexcept;
    bool idempotent(method_id id) noexcept;
}}

#endif
