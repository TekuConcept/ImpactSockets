/**
 * Created by TekuConcept on August 9, 2018
 */

#include "rfc/http/uri.h"

#include "utils/impact_error.h"

using namespace impact;


http::uri::uri()
: impact::uri()
{
    _S_init(&m_parser_options_);
}


http::uri::uri(const impact::uri& r)
: impact::uri(r)
{
    _S_init(&m_parser_options_);
    if (!(m_parser_options_.expect_scheme(scheme()) &&
        m_parser_options_.expect_userinfo(userinfo()) &&
        m_parser_options_.expect_host(host())))
        throw impact_error("Not a valid HTTP URI");
}


http::uri::~uri()
{}


void
http::uri::_S_init(struct uri_parser_opts* __opts)
{
    __opts->expect_scheme =
    [](const std::string& scheme) -> bool {
        return (scheme == "http") || (scheme == "https");
    };
    
    __opts->expect_userinfo =
    [](const std::string& userinfo) -> bool {
        return userinfo.size() == 0;
    };
    
    __opts->expect_host =
    [](const std::string& host) -> bool {
        return host.size() != 0;
    };
}


bool
http::uri::parse(
    std::string  __value,
    impact::uri* __result)
{
    struct uri_parser_opts opts;
    _S_init(&opts);
    return impact::uri::parse(__value, __result, opts);
}


bool
http::uri::parse(
    std::string  __value,
    impact::uri* __result,
    struct uri_parser_opts /* hidden */)
{
    return http::uri::parse(__value, __result);
}
