/**
 * Created by TekuConcept on July 15, 2018
 */

#ifndef _IMPACT_IMPACT_ERRNO_H_
#define _IMPACT_IMPACT_ERRNO_H_

#include <string>

namespace impact {
	enum impact_errors {
		SUCCESS        = 0,

		B64_BADSYM        , /* Bad base-64 symbol */
		B64_BADPAD        , /* Data padding mismatch */
		
		UTF8_BADSYM       , /* Bad unicode symbol */
		UTF8_BADHEAD      , /* Bad unicode byte header */
		UTF8_BADTRAIL     , /* Bad unicode byte trailer */

		URI_SCHEME        , /* No '://' scheme delimiter */
		URI_V6HOST        , /* Bad IPv6 host name */
		URI_HOST_ERR      , /* Bad host name */
		URI_PORT_ERR      , /* Invalid characters used for port */
		URI_PORT_RANGE    , /* Port out of range */
	};

	namespace internal {
		unsigned int& imp_errno_f();
	}
	#define impact_errno internal::imp_errno_f()

	std::string error_string(unsigned int code);
}

#endif
