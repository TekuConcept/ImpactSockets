/**
 * Created by TekuConcept on July 25, 2017
 */

#ifndef _IMPACT_RFC_URI_H_
#define _IMPACT_RFC_URI_H_

#include <string>
#include <sstream>

#define RFC3986 1

namespace impact {
	typedef struct uri_opts {
		std::string query;
		std::string fragment;
		bool        has_authority;
	} UriOptions;
	
	class uri {
	public:
		uri();
		uri(uri&& r);
		uri(std::string value);
		~uri();

		uri& operator=(uri&& r);
		
		static bool parse(std::string value, uri* result);
		
		std::string scheme()    const;
		std::string hier_part() const; // normalized heir_part
		std::string authority() const; // normalized authority
		std::string userinfo()  const; // normalized userinfo
		std::string host()      const; // normalized host
		std::string path()      const; // normalized path
		std::string query()     const; // normalized query
		std::string fragment()  const; // normalized fragment
		std::string str()       const; // normalized str
		std::string abs_str()   const; // normalized abs_str
		int         port()      const;
		
	private:
		struct parser_context {
			size_t             current_idx;
			std::string*       data;
			uri*               result;
		};
		
		std::string  m_scheme_;
		std::string  m_userinfo_;
		std::string  m_host_;
		std::string  m_path_;
		std::string  m_query_;
		std::string  m_fragment_;
		int          m_port_;
		bool         m_has_auth_;
		
		static void _S_clear(struct parser_context*);
		static void _S_path_normalize(std::string*);
		static bool _S_percent_normalize(std::string*,bool);
		static bool _S_parse_uri(struct parser_context*);
		static bool _S_parse_scheme(struct parser_context*);
		static bool _S_parse_hier_part(struct parser_context*);
		static bool _S_parse_authority(struct parser_context*);
		static bool _S_parse_userinfo(struct parser_context*);
		static bool _S_parse_host(struct parser_context*);
		static bool _S_parse_port(struct parser_context*);
		static bool _S_parse_ip_literal(struct parser_context*);
		static bool _S_parse_ipv_future(struct parser_context*);
		static bool _S_parse_ipv6_address(struct parser_context*);
		static bool _S_parse_path(struct parser_context*,bool,bool);
		static bool _S_parse_query(struct parser_context*);
		static bool _S_parse_fragment(struct parser_context*);\
		static inline bool _S_unreserved(char);
		static inline bool _S_reserved(char);
		static inline bool _S_gen_delims(char);
		static inline bool _S_sub_delims(char);
	};
}

#endif
