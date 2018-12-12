/**
 * Created by TekuConcept on July 25, 2017
 *
 * URIs should be immutable objects.
 * Its properties should be readonly.
 */

#include "rfc/uri.h"

#include <sstream>
#include <algorithm>
#include <vector>

#include "utils/environment.h"
#include "utils/impact_error.h"
#include "utils/errno.h"
#include "utils/abnf_ops.h"

#if defined(__OS_WINDOWS__)
    #include <cctype> /* std::toupper */
#endif

#include <iostream>
#include <iomanip>
#define VERB(x) std::cout << x << std::endl

using namespace impact;
using namespace internal; /* abnf_ops */

uri::uri()
: m_port_(-1), m_has_auth_(false), m_has_default_port_(false)
{}


uri::uri(std::string __value)
{
    UNUSED(__value);
    
    struct parser_context context;
    context.current_idx = 0;
    context.data        = &__value;
    context.result      = this;
    
    if (!_S_parse_uri(&context))
        throw impact_error(error_string(imp_errno));
}


uri::~uri() {}


uri::uri(uri&& __value)
: m_scheme_  (std::move(__value.m_scheme_  )),
  m_userinfo_(std::move(__value.m_userinfo_)),
  m_host_    (std::move(__value.m_host_    )),
  m_path_    (std::move(__value.m_path_    )),
  m_query_   (std::move(__value.m_query_   )),
  m_fragment_(std::move(__value.m_fragment_)),
  m_port_    (          __value.m_port_     ),
  m_has_auth_(          __value.m_has_auth_ ),
  m_has_default_port_(__value.m_has_default_port_)
{}


uri&
uri::operator=(uri&& __rvalue)
{
    m_scheme_   = std::move(__rvalue.m_scheme_  );
    m_userinfo_ = std::move(__rvalue.m_userinfo_);
    m_host_     = std::move(__rvalue.m_host_    );
    m_path_     = std::move(__rvalue.m_path_    );
    m_query_    = std::move(__rvalue.m_query_   );
    m_fragment_ = std::move(__rvalue.m_fragment_);
    m_port_     =           __rvalue.m_port_     ;
    m_has_auth_ =           __rvalue.m_has_auth_ ;
    return *this;
}


bool
uri::parse(
    std::string __value,
    uri*        __result)
{
    struct parser_context context;
    context.current_idx = 0;
    context.data        = &__value;
    context.result      = __result;
    
    return _S_parse_uri(&context);
}


std::string uri::scheme()   const { return m_scheme_;   }
std::string uri::userinfo() const { return m_userinfo_; }
std::string uri::host()     const { return m_host_;     }
int uri::port()             const { return m_port_;     }
std::string uri::path()     const { return m_path_;     }
std::string uri::query()    const { return m_query_;    }
std::string uri::fragment() const { return m_fragment_; }

std::string uri::norm_hier_part() const { return _S_percent_decode(hier_part()); }
std::string uri::norm_authority() const { return _S_percent_decode(authority()); }
std::string uri::norm_str()       const { return _S_percent_decode(str());       }
std::string uri::norm_abs_str()   const { return _S_percent_decode(abs_str());   }
std::string uri::norm_userinfo()  const { return _S_percent_decode(m_userinfo_); }
std::string uri::norm_host()      const { return _S_percent_decode(m_host_);     }
std::string uri::norm_path()      const { return _S_percent_decode(m_path_);     }
std::string uri::norm_query()     const { return _S_percent_decode(m_query_);    }
std::string uri::norm_fragment()  const { return _S_percent_decode(m_fragment_); }

std::string
uri::hier_part() const
{
    std::string result = "";
    
    std::string auth = authority();
    if (auth.size() || m_has_auth_) {
        result.append("//");
        result.append(auth);
        
        // RFC3986-3.3, Paragraph 2
        if (m_path_.size())
            result.append("/");
    }
    
    result.append(m_path_);
    
    return result;
}


std::string
uri::authority() const
{
    // authority = [ userinfo "@" ] host [ ":" port ]
    std::string result;
    result.reserve(m_userinfo_.size() + m_host_.size() + 7);
    
    if (m_userinfo_.size()) {
        result.append(m_userinfo_);
        result.append("@");
    }
    
    result.append(m_host_);
    
    // port = *DIGIT
    // which means 0 is a valid uri port number
    // -1 will represent an undetermined / unknown port
    if (m_port_ >= 0 && !m_has_default_port_) {
        result.append(":");
        result.append(std::to_string(m_port_));
    }
    
    return result;
}


std::string
uri::str() const
{
    // RFC3986-5.3:  Component Recomposition
    std::string result = abs_str();

    if (m_fragment_.size()) {
        result.append("#");
        result.append(m_fragment_);
    }

    return result;
}


std::string
uri::abs_str() const
{
    std::string result = "";

    if (m_scheme_.size()) {
        result.append(m_scheme_);
        result.append(":");
    }

    std::string auth = authority();
    if (auth.size() || m_has_auth_) {
        result.append("//");
        result.append(auth);
        
        // RFC3986-3.3, Paragraph 2
        if (m_path_.size() && (m_path_[0] != '/'))
            result.append("/");
    }
    
    result.append(m_path_);
    
    if (m_query_.size()) {
        result.append("?");
        result.append(m_query_);
    }
    
    return result;
}


inline bool
uri::_S_unreserved(char __c)
{
    // unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"
    return ALPHA(__c) || DIGIT(__c)   ||
        (__c == '-')  || (__c == '.') ||
        (__c == '_')  || (__c == '~');
}


inline bool
uri::_S_reserved(char __c)
{
    // reserved = gen-delims / sub-delims
    return _S_gen_delims(__c) || _S_sub_delims(__c);
}


inline bool
uri::_S_gen_delims(char __c)
{
    // gen-delims = ":" / "/" / "?" / "#" / "[" / "]" / "@"
    return
        (__c == ':') || (__c == '/') ||
        (__c == '?') || (__c == '#') ||
        (__c == '[') || (__c == ']') ||
        (__c == '@');
}


inline bool
uri::_S_sub_delims(char __c)
{
    // sub-delims = "!" / "$" / "&" / "'" / "(" / ")"
    //              / "*" / "+" / "," / ";" / "="
    return
        (__c == '!') || (__c == '$')  ||
        (__c == '&') || (__c == '\'') ||
        (__c == '(') || (__c == ')')  ||
        (__c == '*') || (__c == '+')  ||
        (__c == ',') || (__c == ';')  ||
        (__c == '=');
}


void
uri::_S_clear(struct parser_context* __context) {
    if (__context->result) {
        __context->result->m_scheme_   = "";
        __context->result->m_userinfo_ = "";
        __context->result->m_host_     = "";
        __context->result->m_path_     = "";
        __context->result->m_query_    = "";
        __context->result->m_fragment_ = "";
        __context->result->m_port_     = -1;
        __context->result->m_has_auth_ = false;
    }
}


void
uri::_S_path_normalize(std::string* __str)
{
    std::vector<int> dir;
    std::string input_buffer = *__str;
    std::string output_buffer;
    output_buffer.reserve(input_buffer.size());
    
    int dots = 0;
    for (char c : input_buffer) {
        if (c == '.') dots++;
        else if (c == '/') {
            if (dots == 0) {
                output_buffer.push_back(c);
                dir.push_back(output_buffer.size());
            }
            else if (dots == 2) {
                if (dir.size() > 1) {
                    auto i = dir[dir.size() - 2];
                    dir.pop_back();
                    output_buffer.erase(
                        output_buffer.begin() + i,
                        output_buffer.end());
                }
                else if (output_buffer.size() && (output_buffer[0] == '/'))
                    output_buffer.erase(
                        output_buffer.begin() + 1,
                        output_buffer.end());
                else output_buffer.clear();
            }
            else if (dots != 1) {
                while (dots) {
                    output_buffer.push_back('.');
                    dots--;
                }
            }
            dots = 0;
        }
        else {
            if (dots) {
                while (dots) {
                    output_buffer.push_back('.');
                    dots--;
                }
            }
            output_buffer.push_back(c);
        }
    }
    while (dots) {
        output_buffer.push_back('.');
        dots--;
    }
    
    __str->assign(output_buffer);
}


void
uri::_S_post_parse(struct parser_context* __context)
{
    if (__context->result) {
        std::transform(
            __context->result->m_scheme_.begin(),
            __context->result->m_scheme_.end(),
            __context->result->m_scheme_.begin(),
            ::tolower);
        _S_path_normalize(&__context->result->m_path_);
        
        auto& port             = __context->result->m_port_;
        auto scheme            = __context->result->m_scheme_;
        auto& has_default_port = __context->result->m_has_default_port_;
        
        bool scheme_is_registered = true;
        auto token  = s_scheme_port_dictionary_.find(scheme);
        if (token == s_scheme_port_dictionary_.end()) {
            token = s_scheme_port_dictionary_usr_.find(scheme);
            scheme_is_registered =
                (token != s_scheme_port_dictionary_usr_.end());
        }
        
        if (port >= 0)
            has_default_port = (port == token->second);
        else if (scheme_is_registered && (token->second >= 0)) {
            port = token->second;
            has_default_port = true;
        }
        else has_default_port = false;
    }
}


std::string
uri::_S_percent_decode(const std::string& __str) {
    std::string result;
    result.reserve(__str.size());
    
    int state = 0;
    char nibble[2];
    for (char c : __str) {
        if (state == 0) {
            if (c == '%') state++;
            else result.push_back(c);
        }
        else if (state == 1) {
            nibble[0] = c;
            state++;
        }
        else if (state == 2) {
            nibble[1] = c;
            state = 0;
            
            char n = 0;
            for (int i = 0; i < 2; i++) {
                n <<= 4; // size of nibble
                if (nibble[i] >= '0' && nibble[i] <= '9')
                     n |= nibble[i] - '0';
                else n |= nibble[i] - 'A' + 10;
            }
            
            result.push_back(n);
        }
    }
    
    return result;
}


bool
uri::_S_percent_decode(
    std::string* __str,
    bool         __tolower)
{
    if (__str->size() == 0) return true;
    
    imp_errno = imperr::URI_PERCENTENC;
    
    std::istringstream is(*__str);
    std::ostringstream os;
    int state = 0;
    char nibble[2], c;
    
    while (is >> c) {
        if (state == 0) {
            if (c == '%') state++;
            else if (__tolower) os << (char)::tolower(c);
            else os << c;
        }
        else if (state == 1) {
            if (!HEXDIG(c)) {
                imp_errno = imperr::URI_PORT_SYM;
                return false;
            }
            else {
                state++;
                nibble[0] = (char)std::toupper(c);
            }
        }
        else if (state == 2) {
            if (!HEXDIG(c)) {
                imp_errno = imperr::URI_PORT_SYM;
                return false;
            }
            else {
                state = 0;
                nibble[1] = (char)std::toupper(c);
                
                char n = 0;
                for (int i = 0; i < 2; i++) {
                    n <<= 4; // size of nibble
                    if (nibble[i] >= '0' && nibble[i] <= '9')
                         n |= nibble[i] - '0';
                    else n |= nibble[i] - 'A' + 10;
                }
                
                // not decoding '%' will allow for a
                // second %-decoding pass later
                if (_S_reserved(n) || (n == '%'))
                    os << "%" << nibble[0] << nibble[1];
                else os << n;
            }
        }
    }
    
    if (state != 0) {
        imp_errno = imperr::URI_PORT_SYM;
        return false; // % decode not finished
    }
    
    *__str = os.str();
    
    imp_errno = imperr::SUCCESS;
    return true;
}


bool
uri::register_scheme_port(
    std::string __scheme,
    int         __port)
{
    imp_errno = imperr::SUCCESS;
    if (s_scheme_port_dictionary_.find(__scheme) ==
        s_scheme_port_dictionary_.end()) {
        imp_errno = imperr::URI_SCHEME_RSV;
        return false;
    }
    // create or update registered scheme port-value
    s_scheme_port_dictionary_usr_[__scheme] = __port;
    return true;
}


bool
uri::deregister_scheme_port(std::string __scheme)
{
    imp_errno = imperr::SUCCESS;
    auto scheme = s_scheme_port_dictionary_usr_.find(__scheme);
    if (scheme == s_scheme_port_dictionary_usr_.end()) {
        imp_errno = imperr::URI_SCHEME_NOTFOUND;
        return false;
    }
    s_scheme_port_dictionary_usr_.erase(scheme);
    return true;
}


bool
uri::_S_parse_uri(struct parser_context* __context)
{
    // URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
    imp_errno = imperr::SUCCESS;
    
    _S_clear(__context);
    
    std::string& data = *__context->data;
    auto& current_idx = __context->current_idx;
    
    // VERB("[ " << data << " ]");
    
    if (current_idx >= data.size())
        return true;
    
    if (!_S_parse_scheme(__context))
        // impact_errno: see parse_scheme()
        return false; // bad scheme
    
    if (current_idx >= data.size()) {
        imp_errno = imperr::URI_NOCOLON;
        return false; // no ":" delimiter
    }
    
    if (data[current_idx] != ':') {
        imp_errno = imperr::URI_NOCOLON;
        return false; // unrecognized delimiter
    }
    
    current_idx++;
    
    if (!_S_parse_hier_part(__context))
        // impact_errno: see parse_heir_part()
        return false; // bad authority or path
    
    if ((current_idx < data.size()) &&
        (data[current_idx] == '?')) {
        current_idx++;
        if (!_S_parse_query(__context))
            // impact_errno: see parse_query()
            return false; // bad query
    }
    
    if ((current_idx < data.size()) &&
        (data[current_idx] == '#')) {
        current_idx++;
        if (!_S_parse_fragment(__context))
            // impact_errno: see parse_fragment()
            return false; // bad fragment
    }
    
    _S_post_parse(__context);
    
    return true;
}


bool
uri::_S_parse_scheme(struct parser_context* __context)
{
    // scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
    std::string& data = *__context->data;
    auto& current_idx = __context->current_idx;
    size_t last_idx   = current_idx;
    
    char c = data[current_idx];
    if (!ALPHA(c)) {
        imp_errno = imperr::URI_SCHEME;
        return false; // invalid scheme character
    }
    current_idx++;
    
    for (; current_idx < data.size(); current_idx++) {
        c = data[current_idx];
        if (!(ALPHA(c) || DIGIT(c) || (c == '+') || (c == '-') || (c == '.')))
            break;
    }
    
    if (__context->result)
        __context->result->m_scheme_.assign(
            data.begin() + last_idx,
            data.begin() + current_idx);
    
    return true;
}


bool
uri::_S_parse_hier_part(struct parser_context* __context)
{
    /*
    hier-part = "//" authority path-abempty
                 / path-absolute
                 / path-rootless
                 / path-empty
    */
    
    std::string& data = *__context->data;
    auto& current_idx = __context->current_idx;
    
    if (current_idx >= data.size())
        return true; // path-empty
    
    if (data[current_idx] == '/') {
        current_idx++;
        if (current_idx >= data.size()) {
            if (__context->result)
                __context->result->m_path_.push_back('/');
            return true; // path-absolute
        }
        else if (data[current_idx] == '/') {
            current_idx++;
            // impact_errno: see parse_authority()
            bool success = _S_parse_authority(__context);
            if (success)
                success &= _S_parse_path(__context, true, true);
            return success;
        }
        else {
            if (__context->result)
                __context->result->m_path_.push_back('/');
            // impact_errno: see parse_path()
            return _S_parse_path(__context, false, true);
        }
    }
    else {
        char c = data[current_idx];
        if (_S_unreserved(c) || _S_sub_delims(c) ||
            (c == ':') || (c == '@') || (c == '%'))
            // impact_errno: see parse_path()
            return _S_parse_path(__context, false, true);
        else return true; // path-empty
    }
    
    return true;
}


bool
uri::_S_parse_path(
    struct parser_context* __context,
    bool                   __expect_root,
    bool                   __allow_colon)
{
    /*
    path-abempty  = *( "/" segment )
    path-absolute = "/" [ segment-nz *( "/" segment ) ]
    path-noscheme = segment-nz-nc *( "/" segment )
    path-rootless = segment-nz *( "/" segment )
    path-empty    = 0<pchar>
    */
    
    std::string& data = *__context->data;
    auto& current_idx = __context->current_idx;
    size_t last_idx = current_idx;
    
    if (current_idx >= data.size()) return true; // empty path
    
    char c = data[current_idx];
    if (__expect_root) {
             if (c == '?') return true; // empty path, next: query
        else if (c == '#') return true; // empty path, next: fragment
        else if (c != '/') {
            imp_errno = imperr::URI_PATHABEMPTY;
            return false;
        }
        else {
            current_idx++;
            if (current_idx >= data.size()) {
                if (__context->result)
                    __context->result->m_path_.assign("/");
                return true; // empty root
            }
        }
    }
    
    c = data[current_idx];
    if (c == '/') {
        imp_errno = imperr::URI_PATHDELIM;
        return false; // expected rootless path
    }
    else if (c == ':') {
        if (__allow_colon)
            current_idx++;
        else {
            imp_errno = imperr::URI_COLON;
            return false; // expected no-scheme path
        }
    }
    
    // keep track of pchar count between '/'
    size_t pchar_count = 0;
    while (current_idx < data.size()) {
        c = data[current_idx];
        if (_S_unreserved(c) || _S_sub_delims(c) ||
            (c == ':') || (c == '@') || (c == '%')) {
            current_idx++;
            pchar_count++;
        }
        else if (c == '/') {
            // path child node
            if (pchar_count == 0) {
                imp_errno = imperr::URI_AUTHINPATH;
                return false; // found illegal "//" in path
            }
            else {
                current_idx++;
                pchar_count = 0;
            }
        }
        else break; // non-path character found
    }
    
    std::string path(data.begin() + last_idx, data.begin() + current_idx);
    // impact_errno: see percent_decode()
    bool success = _S_percent_decode(&path, false);
    
    if (__context->result)
        __context->result->m_path_.append(path);
    
    return success;
}


bool
uri::_S_parse_authority(struct parser_context* __context)
{
    std::string& data = *__context->data;
    auto& current_idx = __context->current_idx;
    size_t last_idx = current_idx;
    
    auto at_count          = 0;
    auto colon_count_after = 0;
    auto ip_lit_a_count    = 0;
    auto ip_lit_b_count    = 0;
    
    auto at_index         = last_idx - 1;
    auto port_colon_index = last_idx - 1;
    auto ip_lit_a_index   = last_idx - 1;
    auto ip_lit_b_index   = last_idx - 1;
    
    char c;
    while (current_idx < data.size()) {
        c = data[current_idx];
        if (c == '@') {
            at_count++;
            // can only be one '@'
            if (at_count > 1) {
                imp_errno = imperr::URI_MULTI_AT;
                return false;
            }
            else at_index = current_idx;
        }
        else if (c == ':') {
            if (at_count) colon_count_after++;
            port_colon_index = current_idx;
        }
        else if (c == '[') {
            ip_lit_a_count++;
            if (ip_lit_a_count > 1) {
                imp_errno = imperr::URI_MULTI_IP_LIT;
                return false;
            }
            else ip_lit_a_index = current_idx;
        }
        else if (c == ']') {
            ip_lit_b_count++;
            if (ip_lit_b_count > 1) {
                imp_errno = imperr::URI_MULTI_IP_LIT;
                return false;
            }
            else ip_lit_b_index = current_idx;
        }
        else {
            if (!(_S_unreserved(c) || _S_sub_delims(c) || (c == '%')))
                break;
        }
        current_idx++;
    }
    
    if (ip_lit_a_count != ip_lit_b_count) {
        imp_errno = imperr::URI_IP_LIT_MISMATCH;
        return false; // mismatching '[' and ']'
    }
    if (ip_lit_a_index > ip_lit_b_index) {
        imp_errno = imperr::URI_IP_LIT_MISMATCH;
        return false; // misorder ']'...'['
    }
    if (!ip_lit_a_count && (at_count && colon_count_after > 1)) {
        imp_errno = imperr::URI_COLON;
        return false; // too many ':'
    }
    
    std::string userinfo = "";
    std::string host     = "";
    std::string port     = "";
    
    if (at_index >= last_idx) {
        userinfo.assign(
            data.begin() + last_idx,
            data.begin() + at_index);
        last_idx = at_index + 1; // skip '@'
    }
    
    if (ip_lit_a_count) { // host is ip-literal
        if ((port_colon_index > ip_lit_b_index)) {
            if ((port_colon_index - 1) != ip_lit_b_index) {
                imp_errno = imperr::URI_COLON;
                return false; // [::]foo: <- invalid port placement
            }
            port.assign(
                data.begin() + port_colon_index + 1, // ignore ':'
                data.begin() + current_idx);
        }
        host.assign(
            data.begin() + ip_lit_a_index,
            data.begin() + ip_lit_b_index + 1);
    }
    else {
        auto host_index = at_count ? (at_index + 1) : last_idx;
        if (port_colon_index >= last_idx) {
            port.assign(
                data.begin() + port_colon_index + 1,
                data.begin() + current_idx);
            host.assign(
                data.begin() + host_index,
                data.begin() + port_colon_index);
        }
        else {
            host.assign(
                data.begin() + host_index,
                data.begin() + current_idx);
        }
    }
    
    struct parser_context temp;
    bool success     = true;
    temp.result      = __context->result;
    
    // impact_errno: see parse_userinfo()
    temp.current_idx = 0;
    temp.data        = &userinfo;
    success         &= _S_parse_userinfo(&temp);
    
    // impact_errno: see parse_host()
    temp.current_idx = 0;
    temp.data        = &host;
    success         &= _S_parse_host(&temp);
    
    // impact_errno: see parse_port()
    temp.current_idx = 0;
    temp.data        = &port;
    success         &= _S_parse_port(&temp);
    
    if (__context->result)
        __context->result->m_has_auth_ = true;
    
    return success;
}


bool
uri::_S_parse_userinfo(struct parser_context* __context)
{
    // userinfo = *( unreserved / pct-encoded / sub-delims / ":" )
    // assuming data only contains userinfo data
    std::string& data = *__context->data;
    auto& current_idx = __context->current_idx;
    
    char c; // validate against userinfo ABNF
    for (; current_idx < data.size(); current_idx++) {
        c = data[current_idx];
        if (!(_S_unreserved(c) || _S_sub_delims(c) ||
            (c == '%') || (c == ':'))) {
            imp_errno = imperr::URI_USERINFO_SYM;
            return false;
        }
    }
    
    // impact_errno: see percent_decode()
    bool success = _S_percent_decode(&data, false);
    
    if (__context->result)
        __context->result->m_userinfo_ = data;
    
    return success;
}


bool
uri::_S_parse_host(struct parser_context* __context)
{
    (void)__context;
    // host = IP-literal / IPv4address / reg-name
    // reg-name = *( unreserved / pct-encoded / sub-delims )
    /*
    IPv4address is a match subset of reg-name
    Match reg-name first, then match IPv4address later if needed
    (basic_socket will try to resolve the host name either way)
    */
    
    // assuming data only contains host data
    std::string& data = *__context->data;
    auto& current_idx = __context->current_idx;
    
    if (data.size() == 0)
        return true; // empty host allowed (reg-name)
    else if (data[current_idx] == '[') {
        // impact_errno: see parse_ip_literal()
        if(!_S_parse_ip_literal(__context))
            return false;
    }
    else {
        char c; // validate against reg-name ABNF (includes IPv4address)
        for (; current_idx < data.size(); current_idx++) {
            c = data[current_idx];
            if (!(_S_unreserved(c) || _S_sub_delims(c) || (c == '%'))) {
                imp_errno = imperr::URI_HOST_SYM;
                return false;
            }
        }
    }
    
    // impact_errno: see percent_decode()
    bool success = _S_percent_decode(&data, true);

    if (__context->result)
        __context->result->m_host_ = data;
    
    return success;
}


bool
uri::_S_parse_port(struct parser_context* __context)
{
    // port = *DIGIT
    // assuming data only contains port-specific data
    std::string& data = *__context->data;
    auto& current_idx = __context->current_idx;
    
    char c; // validate against port ABNF
    for (; current_idx < data.size(); current_idx++) {
        c = data[current_idx];
        if (!DIGIT(c)) {
            imp_errno = imperr::URI_PORT_SYM;
            return false;
        }
    }
    
    // max(int) is 10 digits, play it safe with 9 digits
    // typically socket ports are only 5 digits in length
    // but the ABNF for URIs allow for any number of digits
    if (current_idx > 9) {
        imp_errno = imperr::URI_PORT_LIMIT;
        return false;
    }
    
    if (__context->result && data.size()) {
        __context->result->m_port_ = std::stoi(data);
    }
    
    return true;
}


bool
uri::_S_parse_ip_literal(struct parser_context* __context)
{
    // IP-literal = "[" ( IPv6address / IPvFuture  ) "]"
    // assuming data only contains ip-literal-specific data
    // also assumes only one each '[' and ']'
    std::string& data = *__context->data;
    imp_errno = imperr::URI_INVL_IP_LIT;
    
    if (data.size() < 4)
        return false; // requires [::] minimum
    
    if (!((data[0] == '[') && (data[data.size()-1] == ']')))
        return false;
    
    imp_errno = imperr::SUCCESS;
    
    std::string d2 = data.substr(1, data.size() - 2);
    struct parser_context context;
    context.current_idx = 0;
    context.data        = &d2;
    context.result      = __context->result;
    
    if (d2[0] == 'v')
        // impact_errno: see parse_ipv_future()
        return _S_parse_ipv_future(&context);
    else
        // impact_errno: see parse_ipv6_address()
        return _S_parse_ipv6_address(&context);
}


bool
uri::_S_parse_ipv_future(struct parser_context* __context)
{
    // IPvFuture = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
    // assuming data only contains IPvfuture-specific data
    std::string& data = *__context->data;
    size_t current_idx = 0;
    imp_errno = imperr::URI_INVL_IP_LIT;
    
    if (data.size() < 4) return false; // at least v0.0
    
    if (data[current_idx] != 'v') return false;
    current_idx++;
    
    if (!HEXDIG(data[current_idx])) return false;
    
    char c;
    do {
        current_idx++;
        c = data[current_idx];
        if (!HEXDIG(c)) break;
    } while(current_idx < (data.size() - 2));
    
    if (data[current_idx] != '.') return false;
    current_idx++;
    
    c = data[current_idx];
    if (!(_S_unreserved(c) || _S_sub_delims(c) || (c == ':')))
        return false;
    current_idx++;
    
    while (current_idx < data.size()) {
        c = data[current_idx];
        if (!(_S_unreserved(c) || _S_sub_delims(c) || (c == ':')))
            return false;
        current_idx++;
    }
    
    imp_errno = imperr::SUCCESS;
    return true;
}


bool
uri::_S_parse_ipv6_address(struct parser_context* __context)
{
    // A long and annoying ipv6 parser
    std::string& data = *__context->data;
    imp_errno = imperr::URI_INVL_IP_LIT;
    
    // parsing backwards makes things easier
    if (data.size() == 0 || data.size() == 1)
        return false; // at least 2 chars required
    size_t current_idx     = data.size() - 1;
    bool have_placeholder  = false;
    bool have_ipv4         = false;
    
    if (data[current_idx  ] == ':' &&
        data[current_idx-1] == ':') { // "::"
        if (current_idx == 1) return true; // no more chars
        have_placeholder = true;
        current_idx -= 2; // move past "::"
        goto hex;
    }
    else if (HEXDIG(data[current_idx])) {
        if (data.size() < 9) { // ::0.0.0.0
            if (data.size() < 3) return false; // at least ::0
            goto hex;
        }
        else {
            for (int i = 1; i <= 3; i++)
                if (data[current_idx - i] == '.')
                    goto dec;
            goto hex;
        }
    }
    else return false;
    
    dec: { // expect "dec.dec.dec.dec"
        int state = 5;
        int dcnt = 0;
        int dgrp = 1;
        char c;
        have_ipv4 = true;
        
        while (current_idx > 1) {
            c = data[current_idx];
            if (state == 5) {
                if (dcnt == 0) {
                         if (c >= '0' && c <= '5')   dcnt++;
                    else if (c >= '6' && c <= '9') { dcnt++; state = 4; }
                    else return false;
                }
                else if (dcnt == 1) {
                         if (c == '5')               dcnt++;
                    else if (c >= '0' && c <= '4') { dcnt++; state = 4; }
                    else if (c >= '6' && c <= '9') { dcnt++; state = 3; }
                    else if (c == '.')             { dcnt = 0; dgrp++; }
                    else if (c == ':')               break;
                    else return false;
                }
                else if (dcnt == 2) {
                         if (c == '2')               dcnt++;
                    else if (c == '.')             { dcnt = 0; dgrp++; }
                    else if (c == ':')               break;
                    else return false;
                }
                else {
                         if (c == '.')             { dcnt = 0; dgrp++; }
                    else if (c == ':')               break;
                    else return false;
                }
            }
            else if (state == 4) {
                if (dcnt == 1) {
                         if (c >= '0' && c <= '4')   dcnt++;
                    else if (c >= '5' && c <= '9') { dcnt++;           state = 3; }
                    else if (c == '.')             { dcnt = 0; dgrp++; state = 5; }
                    else if (c == ':')               break;
                    else return false;
                }
                else if (dcnt == 2) {
                         if (c == '2' || c == '1')   dcnt++;
                    else if (c == '.')             { dcnt = 0; dgrp++; state = 5; }
                    else if (c == ':')               break;
                    else return false;
                }
                else {
                         if (c == '.')             { dcnt = 0; dgrp++; state = 5; }
                    else if (c == ':')               break;
                    else return false;
                }
            }
            else if (state == 3) {
                if (dcnt == 2) {
                         if (c == '1')               dcnt++;
                    else if (c == '.')             { dcnt = 0; dgrp++; state = 5; }
                    else if (c == ':')               break;
                    else return false;
                }
                else {
                         if (c == '.')             { dcnt = 0; dgrp++; state = 5; }
                    else if (c == ':')               break;
                    else return false;
                }
            }
            else return false;
            current_idx--;
        }
        
        if (dgrp != 4) return false;
        
        // guaranteed at least 2 chars left
        // Note: at least 9 chars going into while loop
        //       at least 2 chars comming out of the loop
        if (data[current_idx] == ':') {
            current_idx--;
            if (data[current_idx] == ':') {
                have_placeholder = true;
                if (current_idx == 0)
                    return true; // no more chars
                current_idx--;
            }
        }
        else return false;
    }
    
    hex: {
        // starts with hex because initial ":" checks have been made already
        // guaranteed at least 1 char available
        
        int hcnt     = 0;
        int hgrp     = 0;
        bool p_idx_z = false; // placeholder index is zero
        char c;
        
        do {
            c = data[current_idx];
            
            if (HEXDIG(c)) {
                hcnt++;
                if (hcnt > 4) return false;
            }
            else if (c == ':') {
                if (hcnt > 0) {
                    hcnt = 0;
                    if (hgrp == 8) break;
                    hgrp++;
                }
                else if (hcnt == 0) {
                    if (have_placeholder) return false;
                    else {
                        have_placeholder = true;
                        p_idx_z = current_idx == 0;
                        hgrp--; // avoid counting twice
                    }
                }
                else return false;
            }
            else return false;
            
            if (current_idx == 0) break;
            else current_idx--;
        } while (true);
        hgrp++; // include last group
        
        if (current_idx != 0)
            return false; // bad ipv6
        
        if (!p_idx_z && data[current_idx] == ':')
            return false; // begins with ":x"
        
        if (have_placeholder) {
            if (have_ipv4 && hgrp > 5)       return false; // group count
            else if (!have_ipv4 && hgrp > 7) return false; // group count
        }
        else {
            if (have_ipv4 && hgrp != 6)      return false; // group count
            else if (hgrp != 8)              return false; // group count
        }
    }
    
    imp_errno = imperr::SUCCESS;
    return true;
}


bool
uri::_S_parse_query(struct parser_context* __context)
{
    // query = *( pchar / "/" / "?" )
    std::string& data = *__context->data;
    auto& current_idx = __context->current_idx;
    size_t last_idx = current_idx;
    
    char c;
    while (current_idx < data.size()) {
        c = data[current_idx];
        if (!(_S_unreserved(c) || _S_sub_delims(c) ||
            (c == '%') || (c == ':') || (c == '@') ||
            (c == '/') || (c == '?'))) break;
        current_idx++;
    }
    
    std::string result(data.begin() + last_idx, data.begin() + current_idx);
    // impact_errno: see percent_decode()
    bool success = _S_percent_decode(&result, false);
    
    if (__context->result)
        __context->result->m_query_.assign(result);
    
    return success;
}


bool
uri::_S_parse_fragment(struct parser_context* __context)
{
    // fragment = *( pchar / "/" / "?" )
    
    /* RFC3986-3.5, Paragraph 1
    A fragment identifier component is indicated by the presence of a
    number sign ("#") character and terminated by the end of the URI.
    */ // in other words, match all characters to end of string
    
    std::string& data = *__context->data;
    auto& current_idx = __context->current_idx;
    bool success = true;
    
    if (current_idx < data.size()) {
        std::string result(data.begin() + current_idx, data.end());
        // impact_errno: see percent_decode()
        success = _S_percent_decode(&result, false);
        
        if (__context->result)
            __context->result->m_fragment_.assign(result);
    }
    
    return success;
}


typedef std::pair<std::string,int> scheme_port;
std::map<std::string,int> uri::s_scheme_port_dictionary_{
    scheme_port("aaa"  ,         3868), // Diameter Protocol
    scheme_port("aaas" ,         5658), // Diameter Protocol with Secure Transport
    scheme_port("about",         2019), // about
    scheme_port("acap",           674), // application configuration access protocol
    scheme_port("acct",            -1), // acct
    scheme_port("cap",           1026), // Calendar Access Protocol
    scheme_port("cid",             -1), // content identifier
    scheme_port("coap",          5683), // coap
    scheme_port("coap+tcp",      5683), // coap+tcp
    scheme_port("coap+ws",       5683), // coap+ws
    scheme_port("coaps",         5684), // coaps
    scheme_port("coaps+tcp",     5684), // coaps+tcp
    scheme_port("coaps+ws",      5684), // coaps+ws
    scheme_port("crid",            -1), // TV-Anytime Content Reference Identifier
    scheme_port("data",            -1), // data
    scheme_port("dav",             -1), // dav
    scheme_port("dict",          2628), // dictionary service protocol
    scheme_port("dns",             -1), // Domain Name System
    scheme_port("example",         -1), // example
    scheme_port("file",            -1), // Host-specific file names
    scheme_port("ftp",             21), // File Transfer Protocol
    scheme_port("geo",             -1), // Geographic Locations
    scheme_port("go",            1096), // go
    scheme_port("gopher",          70), // The Gopher Protocol
    scheme_port("h323",            -1), // H.323
    scheme_port("http",            80), // Hypertext Transfer Protocol
    scheme_port("https",          443), // Hypertext Transfer Protocol Secure
    scheme_port("iax",           4569), // Inter-Asterisk eXchange Version 2
    scheme_port("icap",          1344), // Internet Content Adaptation Protocol
    scheme_port("im",              -1), // Instant Messaging
    scheme_port("imap",           143), // internet message access protocol
    scheme_port("info",            -1), // Information Assets with Identifiers in Public Namespaces.
    scheme_port("ipp",            631), // Internet Printing Protocol
    scheme_port("ipps",           631), // Internet Printing Protocol over HTTPS
    scheme_port("iris",            -1), // Internet Registry Information Service
    scheme_port("iris.beep",      702), // iris.beep
    scheme_port("iris.lwz",       715), // iris.lwz
    scheme_port("iris.xpc",       713), // iris.xpc
    scheme_port("iris.xpcs",      714), // iris.xpcs
    scheme_port("jabber",          -1), // perm/jabber jabber
    scheme_port("ldap",           389), // Lightweight Directory Access Protocol
    scheme_port("mailto",          -1), // Electronic mail address
    scheme_port("mid",             -1), // message identifier
    scheme_port("msrp",          2855), // Message Session Relay Protocol
    scheme_port("msrps",         2855), // Message Session Relay Protocol Secure
    scheme_port("mtqp",          1038), // Message Tracking Query Protocol
    scheme_port("mupdate",       3905), // Mailbox Update (MUPDATE) Protocol
    scheme_port("news",          2009), // USENET news
    scheme_port("nfs",           2049), // network file system protocol
    scheme_port("ni",              -1), // ni
    scheme_port("nih",             -1), // nih
    scheme_port("nntp",           119), // USENET news using NNTP access
    scheme_port("opaquelocktoken", -1), // opaquelocktokent
    scheme_port("pkcs11",          -1), // PKCS#11
    scheme_port("pop",            110), // Post Office Protocol v3
    scheme_port("pres",            -1), // Presence
    scheme_port("reload",        6084), // reload
    scheme_port("reload-config", 6084), // reload
    scheme_port("rtsp",           554), // Real-Time Streaming Protocol (RTSP)
    scheme_port("rtsps",          322), // Real-Time Streaming Protocol (RTSP) over TLS
    scheme_port("rtspu",           -1), // Real-Time Streaming Protocol (RTSP) over UDP (RTSPv2: not-implenented)
    scheme_port("service",         -1), // service location
    scheme_port("session",         -1), // session
    scheme_port("shttp",           80), // Secure Hypertext Transfer Protocol
    scheme_port("sieve",         4190), // ManageSieve Protocol
    scheme_port("sip",           5060), // session initiation protocol
    scheme_port("sips",          5061), // secure session initiation protocol
    scheme_port("sms",             -1), // Short Message Service
    scheme_port("snmp",           161), // Simple Network Management Protocol
    scheme_port("soap.beep",       -1), // soap.beep
    scheme_port("soap.beeps",      -1), // soap.beeps
    scheme_port("stun",          3478), // stun
    scheme_port("stuns",         5349), // stuns
    scheme_port("tag",             -1), // tag
    scheme_port("tel",             -1), // telephone
    scheme_port("telnet",          23), // Reference to interactive sessions
    scheme_port("tftp",            69), // Trivial File Transfer Protocol
    scheme_port("thismessage",     -1), // perm/thismessage multipart/related relative reference resolution
    scheme_port("tip",           3372), // Transaction Internet Protocol
    scheme_port("tn3270",          23), // Interactive 3270 emulation sessions
    scheme_port("turn",          3478), // turn
    scheme_port("turns",         5349), // turns
    scheme_port("tv",              -1), // TV Broadcasts
    scheme_port("urn",             -1), // Uniform Resource Names
    scheme_port("vemmi",          575), // versatile multimedia interface
    scheme_port("vnc",           5900), // Remote Framebuffer Protocol
    scheme_port("ws",              80), // WebSocket connections
    scheme_port("wss",            443), // Encrypted WebSocket connections
    scheme_port("xcon",            -1), // xcon
    scheme_port("xcon-userid",     -1), // xcon-userid
    scheme_port("xmlrpc.beep",    602), // xmlrpc.beep
    scheme_port("xmlrpc.beeps",   602), // xmlrpc.beeps
    scheme_port("xmpp",            -1), // Extensible Messaging and Presence Protocol
    scheme_port("z39.50r",        210), // Z39.50 Retrieval
    scheme_port("z39.50s",        210), // Z39.50 Session
};


std::map<std::string,int> uri::s_scheme_port_dictionary_usr_;