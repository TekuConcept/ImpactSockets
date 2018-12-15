/**
 * Created by TekuConcept on December 15, 2018
 */

#ifndef _IMPACT_HTTP_PARSER_
#define _IMPACT_HTTP_PARSER_

#include <string>
#include "rfc/http/types.h"

namespace impact {
namespace http {
    int parse_start_line(const std::string& input, struct start_line* result);
}}

#endif
