/**
 * Created by TekuConcept on March 7, 2021
 */

#include <iostream>
#include <gnutls/gnutls.h>
#include "utils/impact_error.h"

namespace impact {

    class gnutls_compatibility {
    public:
        gnutls_compatibility() {
            const char* version;
            int result = gnutls_global_init();
            if (result < 0)
                throw impact::impact_error(gnutls_strerror(result));
            version = gnutls_check_version("3.4.6");
            if (version == NULL)
                throw impact::impact_error("GnuTLS 3.4.6 or later required\n");
            else std::cout << "GnuTLS Version: " << version << std::endl;
        }
        ~gnutls_compatibility() { gnutls_global_deinit(); }
    };

    extern gnutls_compatibility s_gnutls_compatibility;
    gnutls_compatibility s_gnutls_compatibility;

} /* namespace impact */
