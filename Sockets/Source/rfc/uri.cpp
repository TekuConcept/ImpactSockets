/**
 * Created by TekuConcept on July 25, 2017
 *
 * URIs should be immutable objects.
 * Its properties should be readonly.
 */

#include "rfc/uri.h"

#include <sstream>
#include <exception>
#include <stdexcept>
#include <algorithm>

namespace impact {
    const char k_sp = ' ';
}

using namespace impact;

typedef std::pair<std::string, std::pair<bool, unsigned short>> meta_token;
typedef std::pair<bool, unsigned short> meta_value;

std::map<std::string, std::pair<bool,unsigned short>> uri::s_scheme_meta_data_
= {
    meta_token("http",  meta_value(false, (unsigned short)80)),
    meta_token("https", meta_value(true,  (unsigned short)443)),
    meta_token("ws",    meta_value(false, (unsigned short)80)),
    meta_token("wss",   meta_value(true,  (unsigned short)443))
};


uri::uri()
{}


uri::uri(const std::string& __uri)
{
	if (_S_parse(__uri, *this) != parse_error::NONE)
		throw std::runtime_error("uri::uri()\n");
}


std::string
uri::scheme() const
{
    return m_scheme_;
}


std::string
uri::host() const
{
    return m_host_;
}


std::string
uri::resource() const
{
    return m_resource_;
}


unsigned short
uri::port() const
{
    return m_port_;
}


bool
uri::secure() const
{
    return m_secure_;
}


bool
uri::valid(const std::string& __uri)
{
	uri uri_l;
	return _S_parse(__uri, uri_l) == parse_error::NONE;
}


uri
uri::parse(const std::string& __uri)
{
	uri uri_l;
	auto status = _S_parse(__uri, uri_l);
	if (status != parse_error::NONE)
		throw std::runtime_error("uri::parse()\n");
	return uri_l;
}


uri
uri::try_parse(
	const std::string& __uri,
	bool&              __success)
{
	uri uri_l;
	__success = _S_parse(__uri, uri_l) == parse_error::NONE;
	return uri_l;
}


uri::parse_error
uri::_S_parse(
	const std::string& __uri,
	uri&               __result)
{
	if (!_S_parse_scheme(__uri, __result))
		return parse_error::NO_SCHEME_DELIMITER;

	unsigned int idx = __result.m_scheme_.length() + 3; /* "://" */
	if (idx >= __uri.length())
		return parse_error::NO_SCHEME_DELIMITER;
	if (!(__uri[idx - 2] == '/' && __uri[idx - 1] == '/'))
		return parse_error::NO_SCHEME_DELIMITER;

	parse_error status;
	if (__uri[idx] == '[')
		// IPv6 enclosed: "[::]"
		status = _S_parse_ipv6_host(__uri, idx, __result);
	else status = _S_parse_host(__uri, idx, __result);
	if (status != parse_error::NONE)
		return status;

	// idx should either be pointing at ':' for port,
	// '/' for resource name, or the end of uri
	status = _S_parse_port(__uri, idx, __result);
	if (status != parse_error::NONE)
		return status;

	// store what is left into resourceName
	std::ostringstream os;
	while (idx < __uri.length()) {
		if (__uri[idx] == '#')
			break;
		if (__uri[idx] == k_sp)
			os << "%%20";
		else os << __uri[idx];
		idx++;
	}
	__result.m_resource_ = os.str();
	if (__result.m_resource_.length() == 0)
		__result.m_resource_ = "/";

	return parse_error::NONE;
}


bool
uri::_S_parse_scheme(const std::string& __uri, uri& __result)
{
	std::ostringstream os;
	unsigned int
		idx    = 0,
		length = __uri.length();
	bool found_delimiter = false;

	while (idx < length) {
		if (__uri[idx] == ':') {
			found_delimiter = true;
			break;
		}
		else os << ::tolower(__uri[idx]);
		idx++;
	}

	__result.m_scheme_.assign(os.str());

	if (found_delimiter)
		_S_set_meta_info(__result);

	return found_delimiter;
}


void
uri::_S_set_meta_info(uri& __result)
{
	auto meta = s_scheme_meta_data_.find(__result.m_scheme_);
	if (meta != s_scheme_meta_data_.end()) {
		__result.m_secure_ = meta->second.first;
		__result.m_port_   = meta->second.second;
	}
	else {
		__result.m_secure_ = false;
		__result.m_port_   = 0;
	}
}


uri::parse_error
uri::_S_parse_ipv6_host(
	const std::string& __uri,
	unsigned int&      __offset,
	uri&               __result)
{
	const int k_max_host   = 39; // IPv6 fully exapanded with ':' is 39 chars
	const int k_max_label  = 4; // labels are only 4 hex chars long
	const int k_min_length = 4; // "[::]"
	if ((__uri.length() - __offset) < k_min_length)
		return parse_error::BAD_IPV6_HOST;

	std::ostringstream os;
	int colon_delimiter_count = 0;
	unsigned int& idx         = __offset;

	if (__uri[idx] != '[')
		return parse_error::BAD_IPV6_HOST; // double check
	os << __uri[idx];
	idx++;

	int
		label_length = 0,
		host_length  = 0;
	while (idx < __uri.length()) {
		char c = (char) ::tolower(__uri[idx]);
		if (c == '/')
			break; // didn't close with ']'
		else if (c == ':') {
			label_length = 0;
			host_length++;
			os << c;

			colon_delimiter_count++;
			// IPv6 doesn't have more than 8 groups
			if (colon_delimiter_count > 7)
				break;
		}
		else if (c == ']') {
			// expecting "::" minimum
			if (colon_delimiter_count < 2)
				break;
			else {
				os << c;
				idx++; // align with _S_parse_ipv4_host() return index
				__result.m_host_ = os.str();
				return parse_error::NONE;
			}
		}
		else if ( // only allow legal hex values
			(c >= 'a' && c <= 'f') || // a-f
			(c >= '0' && c <= '9')) {
			label_length++;
			host_length++;
			os << c;
		}
		else break;

		if (label_length > k_max_label || host_length > k_max_host)
			break;
		idx++;
	}

	return parse_error::BAD_IPV6_HOST;
}


uri::parse_error
uri::_S_parse_host(
	const std::string& __uri,
	unsigned int&      __offset,
	uri&               __result)
{
	const int k_max_host  = 254; // 253 + '.'
	const int k_max_label = 63;
	const int k_min_len   = 1; // "a"
	if ((__uri.length() - __offset) < k_min_len)
		return parse_error::BAD_HOST;

	std::ostringstream os;
	int
		label_length = 0,
		host_length  = 0;
	bool
		first_char   = true;
	unsigned int&
		idx          = __offset;

	while (idx < __uri.length()) {
		char c = (char) ::tolower(__uri[idx]);
		if (first_char) {
			// RFC 952: non-alphanumeric chars as the first char
			// are not allowed.
			if (c == '-')
				return parse_error::BAD_HOST;
			else first_char = false;
		}
		if (c == '/') {
			if (host_length < 1)
				return parse_error::BAD_HOST;
			break;
		}
		else if (c == ':')
			break;
		else if ( // only allow valid host name characters
			(c >= 'a' && c <= 'z') || // a-z
			(c >= '0' && c <= '9') || // 0-9
			(c == '-')) {
			os << c;
			label_length++;
			host_length++;
		}
		else if (c == '.') {
			// end of label
			os << c;
			label_length = 0;
			host_length++;
		}
		else return parse_error::BAD_HOST;

		if (label_length > k_max_label || host_length > k_max_host)
			return parse_error::BAD_HOST;
		idx++;
	}

	__result.m_host_ = os.str();
	return parse_error::NONE;
}


uri::parse_error
uri::_S_parse_port(
	const std::string& __uri,
	unsigned int&      __offset,
	uri&               __result)
{
	unsigned int& idx = __offset;
	std::ostringstream os;

	if (idx < __uri.length() && __uri[idx] == ':') {
		// parse port
		const int k_max_chars = 5; // "65535"
		int char_count = 0;
		idx++; // skip ':'
		while (idx < __uri.length()) {
			if (__uri[idx] == '/')
				break;
			else if (__uri[idx] >= '0' && __uri[idx] <= '9') {
				os << __uri[idx];
				char_count++;
			}
			else return parse_error::BAD_PORT;
			if (char_count > k_max_chars)
				return parse_error::PORT_OUT_OF_RANGE;
			idx++;
		}

		std::string strport = os.str();
		if (strport.length() >= 1) {
			// only try to parse if there is something to parse
			unsigned int port = std::stoi(strport);
			if (port > 65535)
				return parse_error::PORT_OUT_OF_RANGE;
			else __result.m_port_ = static_cast<unsigned short>(port);
		}
	}

	return parse_error::NONE;
}


void
uri::register_scheme(
	const std::string& __name,
	unsigned short     __port,
	bool               __secure)
{
	if (__name.length() == 0)
		return;
	std::string name = __name;
	std::transform(
		name.begin(),
		name.end(),
		name.begin(),
		[](char c) { return (char) ::tolower(c); }
	);
	s_scheme_meta_data_[name] = meta_value(__secure, __port);
}


std::string
uri::_S_error_message(parse_error code)
{
	switch (code) {
	case parse_error::NONE:
		return "Success";
	case parse_error::NO_SCHEME_DELIMITER:
		return "No '://' delimiter found after url scheme";
	case parse_error::BAD_IPV6_HOST:
		return "Bad IPv6 host name";
	case parse_error::BAD_HOST:
		return "Bad host name";
	case parse_error::BAD_PORT:
		return "Invalid characters used for port number";
	case parse_error::PORT_OUT_OF_RANGE:
		return "Port out of range";
	default: return "Unknown error";
	}
}
