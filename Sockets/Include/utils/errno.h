/**
 * Created by TekuConcept on July 15, 2018
 */

#ifndef _IMPACT_IMPACT_ERRNO_H_
#define _IMPACT_IMPACT_ERRNO_H_

#include <string>

namespace impact {
	enum class imperr {
		SUCCESS          = 0,

		B64_BADSYM			, /* Bad base-64 symbol */
		B64_BADPAD			, /* Data padding mismatch */
		
		UTF8_BADSYM			, /* Bad unicode symbol */
		UTF8_BADHEAD		, /* Bad unicode byte header */
		UTF8_BADTRAIL		, /* Bad unicode byte trailer */

		URI_SCHEME			, /* Failed to parse uri scheme */
		URI_COLON           , /* Colon delimiter out of place */
		URI_NOCOLON         , /* No collon delimiter */
		URI_PATHABEMPTY     , /* Path is neither absolute nor empty */
		URI_PATHDELIM       , /* Path delimiter '/' found out of place */
		URI_AUTHINPATH      , /* Unexpected '//' in uri path */
		URI_PERCENTENC      , /* Invalid percent encoding */
		URI_MULTI_AT        , /* More than one '@' found in auth name */
		URI_MULTI_IP_LIT    , /* More than one IP literal identifier found in auth name */
		URI_IP_LIT_MISMATCH , /* '[' and ']' IP literal identifiers missing or out of order */
		URI_USERINFO_SYM    , /* Unexpected character or symbol in userinfo */
		URI_HOST_SYM        , /* Unexpected character or symbol in host */
		URI_PORT_SYM        , /* Unexpected character or symbol in port */
		URI_PORT_LIMIT      , /* Port size unsupported */
		URI_INVL_IP_LIT     , /* Invalid IP literal */
		URI_SCHEME_RSV      , /* Scheme already reserved (permanent scheme) */
		URI_SCHEME_NOTFOUND , /* Registered scheme not found */
	};

	namespace internal {
		imperr& imp_errno_f();
	}
	#define imp_errno internal::imp_errno_f()

	std::string error_string(imperr code);
}

#endif
