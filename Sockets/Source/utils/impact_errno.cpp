/**
 * Created by TekuConcept on July 15, 2018
 */

#include "utils/impact_errno.h"

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
		case SUCCESS:			  return "Success";

		case B64_BADSYM:		  return "Bad base-64 symbol";
		case B64_BADPAD:		  return "Data padding mismatch";
		
		case UTF8_BADSYM:		  return "Bad unicode symbol";
		case UTF8_BADHEAD:		  return "Bad unicode byte header";
		case UTF8_BADTRAIL:		  return "Bad unicode byte trailer";

		case URI_SCHEME:		  return "Failed to parse uri scheme";
		case URI_COLON:           return "Colon delimiter out of place";
		case URI_NOCOLON:         return "No collon delimiter";
		case URI_PATHABEMPTY:     return "Path is neither absolute nor empty";
		case URI_PATHDELIM:       return "Path delimiter '/' found out of place";
		case URI_AUTHINPATH:      return "Unexpected '//' in uri path";
		case URI_PERCENTENC:      return "Invalid percent encoding";
		case URI_MULTI_AT:        return "More than one '@' found in auth name";
		case URI_MULTI_IP_LIT:    return "More than one IP literal identifier found in auth name";
		case URI_IP_LIT_MISMATCH: return "'[' and ']' IP literal identifiers missing or out of order";
		case URI_USERINFO_SYM:    return "Unexpected character or symbol in userinfo";
		case URI_HOST_SYM:        return "Unexpected character or symbol in host";
		case URI_PORT_SYM:        return "Unexpected character or symbol in port";
		case URI_PORT_LIMIT:      return "Port size unsupported";
		case URI_INVL_IP_LIT:     return "Invalid IP literal";

		default: return "Unknown impact error";
		}
	}
}