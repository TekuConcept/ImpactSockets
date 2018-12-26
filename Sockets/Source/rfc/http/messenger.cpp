/**
 * Created by TekuConcept on December 15, 2018
 */

#include "rfc/http/messenger.h"

#include <regex>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include "utils/environment.h"
#include "utils/errno.h"

using namespace impact;

http::transfer_encoding::transfer_encoding(
    std::string __name,
    std::function<std::string(const std::string&)> __encoder)
: m_name_(__name.c_str()), m_encode_(__encoder)
{}
http::transfer_encoding::~transfer_encoding() {}

const case_string& http::transfer_encoding::name() const noexcept { return m_name_; }

http::transfer_encoding
http::transfer_encoding::chunked()
{
    return transfer_encoding(
        "chunked",
        [](const std::string& data) -> std::string {
            std::ostringstream os;
            os << std::hex << data.size() << std::dec;
            // os << *( ";" token [ "=" (token / quoted-string) ] )
            os << "\r\n";
            if (data.size())
                os << data << "\r\n";
            else {
                // os << *( header_field CRLF )
                os << "\r\n";
            }
            return os.str();
        }
    );
}

bool
http::transfer_encoding::less::operator() (
    const transfer_encoding& __lhs,
    const transfer_encoding& __rhs) const
{
    if (__lhs.m_name_ == "chunked") return false;
    else if (__rhs.m_name_ == "chunked") return true;
    else return __lhs.m_name_ < __rhs.m_name_;
}


http::message_header::message_header(
    std::string __name,
    std::string __value)
: m_field_name_(__name), m_field_value_(__value)
{
    // validate name
    // validate value
}
http::message_header::~message_header() {}
const std::string& http::message_header::field_name() const noexcept { return m_field_name_; }
const std::string& http::message_header::field_value() const noexcept { return m_field_value_; }


http::message::message(message_type __type)
: m_type_(__type), m_http_major_(1), m_http_minor_(1),
  m_has_body_(false), m_is_fixed_body_(false)
{}
http::message::message(
    message_type                      __type,
    transfer_encoding::set            __encodings,
    std::function<void(std::string*)> __data_callback)
: m_type_(__type), m_http_major_(1), m_http_minor_(1),
  m_has_body_(true), m_is_fixed_body_(false),
  m_transfer_encodings_(__encodings), m_data_callback_(__data_callback)
{
    if ((m_transfer_encodings_.size() > 0) &&
        ((--m_transfer_encodings_.end())->name() != "chunked"))
        m_transfer_encodings_.insert(transfer_encoding::chunked());

    std::ostringstream field_value;
    size_t i = 0;
    for (const auto& token : m_transfer_encodings_) {
        if (i > 0) field_value << ", ";
        field_value << token.name();
        i++;
    }
    m_headers_.push_back(message_header("Transfer-Encoding", field_value.str()));
}
http::message::~message() {}
http::message_type
http::message::type() const noexcept { return m_type_; }

void
http::message::send(std::ostream& __stream)
{
    if (this->m_type_ == message_type::REQUEST) {
        request_message* start_line = dynamic_cast<request_message*>(this);
        __stream << start_line->method() << " ";
        __stream << start_line->target() << " ";
        __stream << "HTTP/" << m_http_major_ << "." << m_http_minor_ << "\r\n";
    }
    else /* (this->m_type_ == message_type::RESPONSE) */ {
        response_message* start_line = dynamic_cast<response_message*>(this);
        __stream << "HTTP/" << m_http_major_ << "." << m_http_minor_ << " ";
        __stream << std::setfill('0') << std::setw(3);
        __stream << start_line->code() << std::setfill(' ') << " ";
        __stream << start_line->status() << "\r\n";
    }
    
    for (const auto& header : m_headers_) {
        __stream << header.field_name() << ": ";
        __stream << header.field_value() << "\r\n";
    }
    
    __stream << "\r\n"; // end of HTTP headers
    
    if (!m_has_body_) return;
    if (m_is_fixed_body_) {
        __stream << m_data_buffer_;
    }
    else {
        do {
            m_data_buffer_.clear();
            m_data_callback_(&m_data_buffer_);
            std::string out_buffer = m_data_buffer_;
            for (const auto& encoding : m_transfer_encodings_)
                out_buffer = encoding.encode(out_buffer);
            __stream << out_buffer;
        } while(m_data_buffer_.size() > 0);
    }
}



http::request_message::request_message()
: message(message_type::REQUEST)
{}
http::request_message::request_message(
    transfer_encoding::set            __encodings,
    std::function<void(std::string*)> __data_callback)
: message(message_type::REQUEST, __encodings, __data_callback)
{}
http::request_message::~request_message() {}

std::shared_ptr<http::request_message>
http::request_message::create(
    std::string __method,
    std::string __target)
{
    std::shared_ptr<request_message> result =
        std::shared_ptr<request_message>(new request_message());
    result->m_method_ = __method;
    result->m_target_ = __target;
    return result;
}

std::shared_ptr<http::request_message>
http::request_message::create(
    std::string                       __method,
    std::string                       __target,
    transfer_encoding::set            __encodings,
    std::function<void(std::string*)> __data_callback)
{
    std::shared_ptr<request_message> result = std::shared_ptr<request_message>(
        new request_message(__encodings, __data_callback));
    result->m_method_ = __method;
    result->m_target_ = __target;
    return result;
}

const std::string& http::request_message::method() const noexcept { return m_method_; }
const std::string& http::request_message::target() const noexcept { return m_target_; }



http::response_message::response_message()
: message(message_type::RESPONSE)
{}
http::response_message::~response_message() {}

std::shared_ptr<http::response_message>
http::response_message::create(
    int         __code,
    std::string __status)
{
    std::shared_ptr<response_message> result =
        std::shared_ptr<response_message>(new response_message());
    result->m_code_   = __code;
    result->m_status_ = __status;
    return result;
}

int http::response_message::code() const noexcept { return m_code_; }
const std::string& http::response_message::status() const noexcept { return m_status_; }

// namespace impact {
// namespace http {
// namespace internal {
//     const std::regex k_start_line_regex(
//         "(([!#$%&'*+\\-.^_`|~0-9a-zA-Z]+) ([a-zA-Z0-9+\\-.:\\/_~%!$&'\\(\\)*,;="
//         "@\\[\\]?]+|\\*) (HTTP\\/[0-9]\\.[0-9])\\r\\n)|((HTTP\\/[0-9]\\.[0-9]) "
//         "([0-9][0-9][0-9]) ([\\t \\x21-\\x7E\\x80-\\xFF]*)\\r\\n)");
//     const std::regex k_header_field_regex(
//         "([!#$%&'*+\\-.^_`|~0-9a-zA-Z]+):[ \\t]*((?:[\\x21-\\x7E]+)?(?:[ \\t]+["
//         "!-~]+)*)[ \\t]*\\r\\n");
// }}}

// using namespace impact;

// /*  Note when parsind header fields:
//     - preprend SP to field-value before splitting into tokens with regex
//     - replace detected obs-fold with SP
//     - determine obs-fold line matches field-value semantics before appending
// */

// http::start_line::start_line()
// : type(message_type::REQUEST),
//   method("GET"), target("/"),
//   http_major(1), http_minor(1)
// {}


// http::start_line::~start_line()
// {}


// http::start_line::start_line(const start_line& __rhs)
// {
//     this->type       = __rhs.type;
//     this->http_major = __rhs.http_major;
//     this->http_minor = __rhs.http_minor;
//     if (this->type == message_type::REQUEST) {
//         this->method = __rhs.method;
//         this->target = __rhs.target;
//     }
//     else {
//         this->message = __rhs.message;
//         this->status  = __rhs.status;
//     }
// }


// int
// http::internal::parse_start_line(
//     const std::string& __input,
//     struct start_line* __result)
// {
//     /* match indicies
//         0 - full string match
//         1 - request match
//         2 - request method
//         3 - request target
//         4 - request HTTP version
//         5 - response match
//         6 - response HTTP version
//         7 - response status code
//         8 - response status message
//     */
    
//     if (__result == NULL) {
//         imp_errno = imperr::INVALID_ARGUMENT;
//         return -1;
//     }
//     else imp_errno = imperr::SUCCESS;
    
//     std::smatch match;
//     if (std::regex_match(__input, match, k_start_line_regex)) {
//         // current assumptions are that given 9 matches,
//         // - four of the matches are blank
//         // - three subsequent match indicies following 1 or 5 are not blank
//         if (match.size() != 9) {
//             imp_errno = imperr::HTTP_BAD_MATCH;
//             return -1;
//         }
        
//         // typeof(match[i]) = std::ssub_match
//         std::string http_version;
//         if (match[1].str().size() != 0) {
//             http_version      = match[4].str();
//             __result->method  = match[2].str();
//             __result->target  = match[3].str();
//             __result->type    = message_type::REQUEST;
//         }
//         else if (match[5].str().size() != 0) {
//             http_version      = match[6].str();
//             __result->status  = std::stoi(match[7].str());
//             __result->message = match[8].str();
//             __result->type    = message_type::RESPONSE;
//         }
//         else {
//             imp_errno = imperr::HTTP_BAD_MATCH;
//             return -1;
//         }
        
//         __result->http_major  = http_version[5] - '0';
//         __result->http_minor  = http_version[7] - '0';
//     }
//     else {
//         imp_errno = imperr::HTTP_BAD_MATCH;
//         return -1;
//     }
    
//     return 0;
// }


// int
// http::internal::parse_header_line(
//     const std::string&                  __input,
//     std::pair<std::string,std::string>* __result)
// {
//     /* match indicies
//         0 - full string match
//         1 - field name
//         2 - field value (trimmed)
//     */
    
//     if (__result == NULL) {
//         imp_errno = imperr::INVALID_ARGUMENT;
//         return -1;
//     }
//     else imp_errno = imperr::SUCCESS;
    
//     std::smatch match;
//     if (std::regex_match(__input, match, k_header_field_regex)) {
//         if (match.size() != 3) {
//             imp_errno = imperr::HTTP_BAD_MATCH;
//             return -1;
//         }
//         // typeof(match[i]) = std::ssub_match
//         __result->first  = match[1].str();
//         __result->second = match[2].str();
//     }
//     else {
//         imp_errno = imperr::HTTP_BAD_MATCH;
//         return -1;
//     }
    
//     return 0;
// }


// http::body_format::body_format()
// : m_type_(body_format_type::FIXED)
// {}


// http::body_format::~body_format()
// {}


// http::body_format_type
// http::body_format::type() const noexcept
// {
//     return this->m_type_;
// }


// http::fixed_body::fixed_body()
// : fixed_body("")
// {}


// http::fixed_body::fixed_body(std::string __data)
// : data(__data)
// {}


// http::fixed_body::~fixed_body()
// {}


// http::transfer_encoding::~transfer_encoding()
// {}


// http::dynamic_body::dynamic_body()
// : dynamic_body(NULL, {})
// {}


// http::dynamic_body::dynamic_body(
//     std::iostream*                  __data_stream,
//     std::vector<transfer_encoding*> __encodings)
// : data_stream(__data_stream)
// {
//     // remove deplicate encodings
//     // if has chunked encoding:
//     //     move encoding to back
//     // else append chuncked encoding
//     UNUSED(__encodings);
//     m_type_ = body_format_type::DYNAMIC;
// }


// http::dynamic_body::~dynamic_body()
// {}


// http::message::~message()
// {}


// http::message_type
// http::message::type() const noexcept
// {
//     return m_start_line_.type;
// }


// int
// http::message::http_major() const noexcept
// {
//     return m_start_line_.http_major;
// }


// int
// http::message::http_minor() const noexcept
// {
//     return m_start_line_.http_minor;
// }


// void
// http::message::send(std::ostream* __stream) const
// {
//     if (__stream == NULL) return;
//     auto& out = *__stream;

//     if (this->m_start_line_.type == message_type::REQUEST) {
//         out << this->m_start_line_.method << " ";
//         out << this->m_start_line_.target << " ";
//         out << "HTTP/";
//         out << (int)this->m_start_line_.http_major << ".";
//         out << (int)this->m_start_line_.http_minor << "\r\n";
//     }
//     else {
//         out << "HTTP/";
//         out << (int)this->m_start_line_.http_major << ".";
//         out << (int)this->m_start_line_.http_minor << " ";
//         out << std::setfill('0') << std::setw(3);
//         out << this->m_start_line_.status << std::setfill(' ') << " ";
//         out << this->m_start_line_.message << "\r\n";
//     }
    
//     // do not write any body format headers
//     // except those that match the message body_format
//     for (const auto& header : this->headers) {
//         out << header.first << ": ";
//         out << header.second << "\r\n";
//     }
    
//     out << "\r\n";
    
//     // out << body
// }


// http::request_message::request_message(
//     std::string __method,
//     std::string __target)
// {
//     // validate method: escape invalid characters, toupper
//     // validate target: escape invalid characters

//     this->m_start_line_.type       = message_type::REQUEST;
//     this->m_start_line_.http_major = 1;
//     this->m_start_line_.http_minor = 1;
//     this->m_start_line_.method     = __method;
//     this->m_start_line_.target     = __target;
    
//     std::transform(
//         this->m_start_line_.method.begin(),
//         this->m_start_line_.method.end(),
//         this->m_start_line_.method.begin(),
//         ::toupper
//     );
// }


// http::request_message::~request_message()
// {}


// http::message_type
// http::request_message::type() const noexcept
// {
//     return this->m_start_line_.type;
// }


// std::string
// http::request_message::method() const noexcept
// {
//     return m_start_line_.method;
// }


// std::string
// http::request_message::target() const noexcept
// {
//     return m_start_line_.target;
// }
