/**
 * Created by TekuConcept on July 15, 2018
 */

#ifndef _IMPACT_IMPACT_ERRNO_H_
#define _IMPACT_IMPACT_ERRNO_H_

#include <string>

namespace impact {
	enum impact_errors {
		SUCCESS        = 0,

		B64_BADSYM     = 1, /* Bad base-64 symbol */
		B64_BADPAD     = 2, /* Data padding mismatch */

		URI_SCHEME     = 3, /* No '://' scheme delimiter */
		URI_V6HOST     = 4, /* Bad IPv6 host name */
		URI_HOST_ERR   = 5, /* Bad host name */
		URI_PORT_ERR   = 6, /* Invalid characters used for port */
		URI_PORT_RANGE = 7, /* Port out of range */
	};

	namespace internal {
		unsigned int& imp_errno_f();
	}
	#define impact_errno internal::imp_errno_f()

	std::string error_string(unsigned int code);
}

#endif
