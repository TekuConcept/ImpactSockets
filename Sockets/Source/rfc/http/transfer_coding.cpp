/**
 * Created by TekuConcept on October 4, 2019
 */

#include "rfc/http/transfer_coding.h"

#include <sstream>
#include <iomanip>

#include "utils/impact_error.h"
#include "utils/string_ops.h"
#include "utils/abnf_ops.h"
#include "rfc/http/abnf_ops.h"

using namespace impact;
using namespace http;


transfer_coding::transfer_coding() = default;


transfer_coding::transfer_coding(std::string __name)
{
    impact::internal::trim_whitespace(&__name);
    m_name_.assign(__name.c_str(), __name.size());

    if (m_name_ == "chunked" ||
        m_name_ == "compress" ||
        m_name_ == "deflate" ||
        m_name_ == "gzip") {
        __name.append(" is a reserved name");
        throw impact_error(__name);
    }
    else if (!_M_valid_transfer_extension(__name))
        throw impact_error("invalid transfer-extension name");
}


transfer_coding::~transfer_coding() = default;


bool
transfer_coding::_M_valid_transfer_extension(const std::string& __value) const
{
    /* transfer-extension ABNF
        OWS            = *( SP / HTAB )
                        ; optional whitespace
        RWS            = 1*( SP / HTAB )
                        ; required whitespace
        BWS            = OWS
                        ; "bad" whitespace
        token = 1*tchar
        transfer-extension = token *( OWS ";" OWS transfer-parameter )
        transfer-parameter = token BWS "=" BWS ( token / quoted-string )
        quoted-string = DQUOTE *( dqtext / quoted-pair ) DQUOTE
        qdtext = HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text
        obs-text = %x80-FF
        quoted-pair = "\" ( HTAB / SP / VCHAR / obs-text )
    */

    if (__value.size() == 0) return false;

    // process token until white space
    int state = 0;
    bool in_parameter = false;
    bool quoted_pair = false;
    // std::string result;
    // result.reserve(__value.size());
    for (auto c : __value) {
        switch (state) {
        case 0: // token
            if (!internal::TCHAR(c)) {
                in_parameter = true;
                if (impact::internal::SP(c) ||
                    impact::internal::HTAB(c))
                    state = 1; // OWS-1
                else if (c == ';')
                    state = 2; // OWS-2
                else return false;
            }
            break;
        case 1: // OWS-1
            if (impact::internal::SP(c) ||
                impact::internal::HTAB(c))
                continue; // OWS
            else if (c == ';')
                state = 2;
            else return false;
            break;
        case 2: // OWS-2
            if (impact::internal::SP(c) ||
                impact::internal::HTAB(c))
                continue; // OWS
            else if (internal::TCHAR(c))
                state = 3;
            else return false;
            break;
        case 3: // parameter-token
            if (internal::TCHAR(c))
                continue;
            else if (impact::internal::SP(c) ||
                impact::internal::HTAB(c))
                state = 4; // BWS
            else if (c == '=')
                state = 5;
            else return false;
            break;
        case 4: // BWS-1
            if (impact::internal::SP(c) ||
                impact::internal::HTAB(c))
                continue; // BWS
            else if (c == '=')
                state = 5;
            else return false;
            break;
        case 5: // BWS-2
            if (impact::internal::SP(c) ||
                impact::internal::HTAB(c))
                continue; // BWS
            else if (internal::TCHAR(c)) {
                in_parameter = false;
                state = 0; // token
            }
            else if (c == '"')
                state = 6; // quoted string
            else return false;
            break;
        case 6: // parameter-value-quoted-string
            if (quoted_pair) {
                if (impact::internal::HTAB(c) ||
                    impact::internal::SP(c) ||
                    impact::internal::VCHAR(c) ||
                    internal::OBS_TEXT(c))
                    quoted_pair = false;
                else return false;
            }
            else if (c == '"') {
                in_parameter = false;
                state = 1;
            }
            else if (c == '\\')
                quoted_pair = true;
            else if (
                impact::internal::HTAB(c) ||
                impact::internal::SP(c) ||
                internal::OBS_TEXT(c) ||
                (c == '\x21') ||
                (c >= '\x23' && c <= '\x5B') ||
                (c >= '\x5D' && c <= '\x7E'))
                continue;
            else return false;
            break;
        }
    }

    if (in_parameter) return false;

    return true;
}


chunked_coding::extension_t::extension_t(
    std::string __name,
    std::string __value)
{
    name(__name);
    value(__value);
}

chunked_coding::extension_t::extension_t() = default;


chunked_coding::extension_t::~extension_t() = default;


chunked_coding::extension_t
chunked_coding::extension_t::parse(std::string __raw)
{
    extension_t result;
    if (!_M_parse(__raw, &result))
        throw impact_error("failed to parse chuncked extension");
    return result;
}


bool
chunked_coding::extension_t::_M_parse(
    const std::string& __raw,
    extension_t*       __extension)
{
    if (__raw.size() < 2) return false;
    if (__raw[0] != ';') return false;

    auto split_index = __raw.find_first_of('=', 1);
    if (split_index == __raw.npos) {
        __extension->m_name_ = __raw.substr(1, __raw.npos);
        return internal::is_token(__extension->m_name_);
    }
    else {
        __extension->m_name_  = __raw.substr(1, split_index - 1);
        __extension->m_value_ = __raw.substr(split_index + 1, __raw.npos);
        return
             internal::is_token(__extension->m_name_) &&
            (internal::is_token(__extension->m_value_) ||
             internal::is_quoted_string(__extension->m_value_));
    }
}


void
chunked_coding::extension_t::name(std::string __name)
{
    if (!internal::is_token(__name))
        throw impact_error("name not of type token");
    else m_name_ = __name;
}


void
chunked_coding::extension_t::value(std::string __value)
{
    if (__value.size() == 0) {
        m_value_.clear();
        return;
    }
    if (!internal::is_token(__value) &&
        !internal::is_quoted_string(__value))
        throw impact_error("value not of type token or quoted-string");
    else m_value_ = __value;
}


std::string
chunked_coding::extension_t::to_string() const
{
    std::ostringstream os;
    os << ";" << m_name_;
    if (m_value_.size() > 0)
        os << "=" << m_value_;
    return os.str();
}


namespace impact {
namespace http {
    std::ostream&
    operator<<(
        std::ostream&                      __os,
        const chunked_coding::extension_t& __ext)
    {
        __os << __ext.to_string();
        return __os;
    }
}}


std::set<case_string> chunked_coding::s_forbidden_trailers_ = {
    "Authorizaton", "Cache-Control", "Content-Encoding",
    "Content-Length", "Content-Range", "Content-Type", "Date",
    "Expect", "Expires", "Host", "Location", "Max-Forwards",
    "Pragma", "Proxy-Authenticate", "Proxy-Authorization",
    "Range", "Retry-After", "Set-Cookie", "TE", "Trailer",
    "Transfer-Encoding", "Vary", "Warning", "WWW-Authenticate"
};


chunked_coding::chunked_coding()
: transfer_coding(), m_observer_(NULL)
{ transfer_coding::m_name_ = "chunked"; }


chunked_coding::~chunked_coding() = default;


std::string
chunked_coding::encode(const std::string& __buffer)
{
    std::ostringstream os;
    std::vector<chunked_coding::extension_t> extensions;

    if (__buffer.size() == 0) { // last chunk
        header_list headers;
        if (m_observer_)
            m_observer_->on_last_chunk(extensions, headers);
        os << "0";
        for (const auto& extension : extensions)
            os << extension;
        os << "\r\n";
        for (const auto& header : headers) {
            auto n = s_forbidden_trailers_.find(header.name());
            if (n == s_forbidden_trailers_.end())
                os << header;
        }
        os << "\r\n"; // empty line
    }
    else { // chunk
        if (m_observer_)
            m_observer_->on_next_chunk(extensions, __buffer);
        os << std::hex << std::uppercase << __buffer.size();
        os << std::dec << std::nouppercase;
        for (const auto& extension : extensions)
            os << extension;
        os << "\r\n";
        os << __buffer << "\r\n";
    }

    return os.str();
}


void
chunked_coding::parse_chunk_header(
    const std::string&        __raw,
    size_t*                   __size,
    std::vector<extension_t>* __extensions)
{
    try {
        if (!_M_parse_chunk_header(__raw, __size, __extensions))
            throw impact_error("chunk header not formatted properly");
    }
    catch (...) { throw; }
}


bool
chunked_coding::_M_parse_chunk_header(
    const std::string&        __raw,
    size_t*                   __size,
    std::vector<extension_t>* __extensions)
{
    // 1*HEXDIG *( ";" chunk-ext-name [ "=" chunk-ext-val ] ) CRLF
    if (__raw.size() < 3) return false;
    if (__raw[__raw.size() - 2] != '\r' ||
        __raw[__raw.size() - 1] != '\n') return false;

    size_t size  = 0;
    size_t begin = 0;
    size_t end   = __raw.size() - 2;
    std::vector<extension_t> extensions;

    size_t i            = begin;
    bool found_size     = false;
    bool has_extensions = false;
    bool quoted_pair    = false;

    for (; i <= end; i++) {
        if (__raw[i] == '\r') {
            if (i != end) return false;
            else found_size = true;
            break;
        }
        else if (__raw[i] == ';') {
            found_size = true;
            has_extensions = true;
            begin = i;
            break;
        }
        else if (!impact::internal::HEXDIG(__raw[i]))
            return false;
    }

    if (!found_size) return false;
    else size = std::stoi(__raw.substr(0, i), 0, 16); // will throw if error

    // TODO: use indexed lambdas instead of swiches?
    if (has_extensions) {
        int state = 0;
        for (i = ++begin; i <= end; i++) {
            char c = __raw[i];
            switch (state) {
            case 0: // magic
                if (internal::TCHAR(c)) {
                    extensions.push_back(extension_t());
                    state = 1;
                }
                else return false;
                break;

            case 1: // token
                if (internal::TCHAR(c))
                    continue;
                else if (c == '=') {
                    extensions.back().m_name_ =
                        __raw.substr(begin, i - begin);
                    state = 2;
                    begin = i + 1;
                }
                else if (c == ';') {
                    extensions.back().m_name_ =
                        __raw.substr(begin, i - begin);
                    state = 0;
                    begin = i + 1;
                }
                else if (c == '\r') {
                    if (i != end) return false;
                    extensions.back().m_name_ =
                        __raw.substr(begin, i - begin);
                }
                else return false;
                break;

            case 2: // value
                if (internal::TCHAR(c))
                    state = 3;
                else if (c == '"')
                    state = 4;
                else return false;
                break;

            case 3: // value as token
                if (internal::TCHAR(c))
                    continue;
                else if (c == ';') {
                    extensions.back().m_value_ =
                        __raw.substr(begin, i - begin);
                    state = 0;
                    begin = i + 1;
                }
                else if (c == '\r') {
                    if (i != end) return false;
                    extensions.back().m_value_ =
                        __raw.substr(begin, i - begin);
                }
                else return false;
                break;
            
            case 4: // value as quoted string
                if (quoted_pair) {
                    if (!impact::internal::HTAB(c)  &&
                        !impact::internal::SP(c)    &&
                        !impact::internal::VCHAR(c) &&
                        !internal::OBS_TEXT(c)) return false;
                    quoted_pair = false; // reset
                    continue;
                }
                else if (c == '\\') quoted_pair = true;
                else if (
                    impact::internal::HTAB(c)    ||
                    impact::internal::SP(c)      ||
                    (c == '\x21')                ||
                    (c >= '\x23' && c <= '\x5B') ||
                    (c >= '\x5D' && c <= '\x7E') ||
                    internal::OBS_TEXT(c)) continue;
                else if (c == '"')
                    state = 5;
                else return false;
                break;

            case 5: // end value as quoted string
                if (c == '\r') {
                    if (i != end) return false;
                    extensions.back().m_value_ =
                        __raw.substr(begin, i - begin);
                }
                else if (c == ';') {
                    extensions.back().m_value_ =
                        __raw.substr(begin, i - begin);
                    state = 0;
                    begin = i + 1;
                }
                else return false;
                break;
            }
        }

        if (quoted_pair) return false;
    }

    if (__size) *__size = size;
    if (__extensions) *__extensions = std::move(extensions);

    return true;
}
