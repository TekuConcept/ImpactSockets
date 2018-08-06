/**
 * Created by TekuConcept on July 15, 2018
 */

#include "utils/errno.h"

namespace impact {
	namespace internal {
		thread_local imperr s_imp_errno_ = imperr::SUCCESS;
	}

	imperr&
	internal::imp_errno_f()
	{
		return s_imp_errno_;
	}


	std::string
	error_string(imperr __code)
	{
		switch (__code) {
		case imperr::SUCCESS:			  return "Success";

		case imperr::B64_BADSYM:		  return "Bad base-64 symbol";
		case imperr::B64_BADPAD:		  return "Data padding mismatch";
		
		case imperr::UTF8_BADSYM:		  return "Bad unicode symbol";
		case imperr::UTF8_BADHEAD:		  return "Bad unicode byte header";
		case imperr::UTF8_BADTRAIL:		  return "Bad unicode byte trailer";

		case imperr::URI_SCHEME:		  return "Failed to parse uri scheme";
		case imperr::URI_COLON:           return "Colon delimiter out of place";
		case imperr::URI_NOCOLON:         return "No collon delimiter";
		case imperr::URI_PATHABEMPTY:     return "Path is neither absolute nor empty";
		case imperr::URI_PATHDELIM:       return "Path delimiter '/' found out of place";
		case imperr::URI_AUTHINPATH:      return "Unexpected '//' in uri path";
		case imperr::URI_PERCENTENC:      return "Invalid percent encoding";
		case imperr::URI_MULTI_AT:        return "More than one '@' found in auth name";
		case imperr::URI_MULTI_IP_LIT:    return "More than one IP literal identifier found in auth name";
		case imperr::URI_IP_LIT_MISMATCH: return "'[' and ']' IP literal identifiers missing or out of order";
		case imperr::URI_USERINFO_SYM:    return "Unexpected character or symbol in userinfo";
		case imperr::URI_HOST_SYM:        return "Unexpected character or symbol in host";
		case imperr::URI_PORT_SYM:        return "Unexpected character or symbol in port";
		case imperr::URI_PORT_LIMIT:      return "Port size unsupported";
		case imperr::URI_INVL_IP_LIT:     return "Invalid IP literal";
		case imperr::URI_SCHEME_RSV:      return "Scheme already reserved (permanent scheme)";
		case imperr::URI_SCHEME_NOTFOUND: return "Registered scheme not found";

		default: return "Unknown impact error";
		}
	}
}