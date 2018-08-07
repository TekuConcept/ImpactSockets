/**
 * Created by TekuConcept on July 25, 2017
 */

#ifndef _IMPACT_RFC_URI_H_
#define _IMPACT_RFC_URI_H_

#include <string>
#include <sstream>
#include <map>

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
		virtual ~uri();

		uri& operator=(uri&& r);
		
		static bool register_scheme_port(std::string scheme, int port);
		static bool deregister_scheme_port(std::string scheme);
		static bool parse(std::string value, uri* result);
		
		std::string scheme()         const;
		std::string hier_part()      const;
		std::string authority()      const;
		std::string userinfo()       const;
		std::string host()           const;
		std::string path()           const;
		std::string query()          const;
		std::string fragment()       const;
		std::string str()            const;
		std::string abs_str()        const;
		int         port()           const;
		
		std::string norm_hier_part() const;
		std::string norm_authority() const;
		std::string norm_userinfo()  const;
		std::string norm_host()      const;
		std::string norm_path()      const;
		std::string norm_query()     const;
		std::string norm_fragment()  const;
		std::string norm_str()       const;
		std::string norm_abs_str()   const;
		
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
		bool         m_has_default_port_;
		
		// Based on IANA Registered URI Schemes 08/03/2018 (permanent)
		static std::map<std::string,int> s_scheme_port_dictionary_;
		// User-Defined URI Schemes
		static std::map<std::string,int> s_scheme_port_dictionary_usr_;
		
		static void _S_clear(struct parser_context*);
		static void _S_path_normalize(std::string*);
		static void _S_post_parse(struct parser_context*);
		static std::string _S_percent_decode(const std::string&);
		static bool _S_percent_decode(std::string*,bool);
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
