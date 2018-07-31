/**
 * Created by TekuConcept on July 15, 2018
 */

#include "sockets/impact_errno.h"

namespace impact {
	namespace internal {
		thread_local unsigned int imp_errno = SUCCESS;
	}

	unsigned int&
	internal::imp_errno_f()
	{
		return imp_errno;
	}


	std::string
	error_string(unsigned int __code)
	{
		switch (__code) {
		case SUCCESS:        return "Success";

		case B64_BADSYM:     return "Bad base-64 symbol";
		case B64_BADPAD:     return "Data padding mismatch";
		
		case UTF8_BADSYM:    return "Bad unicode symbol";
		case UTF8_BADHEAD:   return "Bad unicode byte header";
		case UTF8_BADTRAIL:  return "Bad unicode byte trailer";

		case URI_SCHEME:     return "No '://' scheme delimiter";
		case URI_V6HOST:     return "Bad IPv6 host name";
		case URI_HOST_ERR:   return "Bad host name";
		case URI_PORT_ERR:   return "Invalid characters used for port";
		case URI_PORT_RANGE: return "Port out of range";
		
		default: return "Unknown impact error";
		}
	}
}