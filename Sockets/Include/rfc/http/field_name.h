/**
 * Created by TekuConcept on August 13, 2018
 */

#ifndef _IMPACT_HTTP_FIELD_NAME_H_
#define _IMPACT_HTTP_FIELD_NAME_H_

#include <string>

namespace impact {
namespace http {
    typedef enum class field_name_id {
        /* - RFC 7230 - */
        CONNECTION,          // Section 6.1
        CONTENT_LENGTH,      // Section 3.3.2
        HOST,                // Section 5.4
        TE,                  // Section 4.3
        TRAILER,             // Section 4.4
        TRANSFER_ENCODING,   // Section 3.3.1
        UPGRADE,             // Section 6.7
        VIA,                 // Section 5.7.1
        CLOSE,               // Section 8.1 (reserved)

        /* - RFC 7231 - */
        ACCEPT,              // Section 5.3.2
        ACCEPT_CHARSET,      // Section 5.3.3
        ACCEPT_ENCODING,     // Section 5.3.4
        ACCEPT_LANGUAGE,     // Section 5.3.5
        ALLOW,               // Section 7.4.1
        CONTENT_ENCODING,    // Section 3.1.2.2
        CONTENT_LANGUAGE,    // Section 3.1.3.2
        CONTENT_LOCATION,    // Section 3.1.4.2
        CONTENT_TYPE,        // Section 3.1.1.5
        DATE,                // Section 7.1.1.2
        EXPECT,              // Section 5.1.1
        FROM,                // Section 5.5.1
        LOCATION,            // Section 7.1.2
        MAX_FORWARDS,        // Section 5.1.2
        MIME_VERSION,        // Appendix A.1
        REFERER,             // Section 5.5.2
        RETRY_AFTER,         // Section 7.1.3
        SERVER,              // Section 7.4.2
        USER_AGENT,          // Section 5.5.3
        VARY,                // Section 7.1.4
        
        /* - RFC 7232 - */
        ETAG,                // Section 2.3
        IF_MATCH,            // Section 3.1
        IF_MODIFIED_SINCE,   // Section 3.3
        IF_NONE_MATCH,       // Section 3.2
        IF_UNMODIFIED_SINCE, // Section 3.4
        LAST_MODIFIED,       // Section 2.2
        
        /* - RFC 7233 - */
        ACCEPT_RANGES,       // Section 2.3
        CONTENT_RANGE,       // Section 4.2
        IF_RANGE,            // Section 3.2
        RANGE,               // Section 3.1
        
        /* - RFC 7234 - */
        AGE,                 // Section 5.1
        CACHE_CONTROL,       // Section 5.2
        EXPIRES,             // Section 5.3
        PRAGMA,              // Section 5.4
        WARNING,             // Section 5.5
        
        /* - RFC 7235 - */
        AUTHORIZATION,       // Section 4.2
        PROXY_AUTHENTICATE,  // Section 4.3
        PROXY_AUTHORIZATION, // Section 4.4
        WWW_AUTHENTICATE,    // Section 4.1
    } FieldNameID;
    
    std::string field_name_str(field_name_id id) noexcept;
}}

#endif
