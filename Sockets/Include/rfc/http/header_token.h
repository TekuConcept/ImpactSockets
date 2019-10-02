/**
 * Created by TekuConcept on December 28, 2018
 */

#ifndef _IMPACT_HTTP_HEADER_TOKEN_H_
#define _IMPACT_HTTP_HEADER_TOKEN_H_

#include <string>
#include "rfc/http/types.h"
#include "utils/case_string.h"

namespace impact {
namespace http {
    // caching object
    // (speed up execution by reducing validation checks)
    class header_token {
    public:
        header_token(std::string line);
        header_token(std::string name, std::string value);
        header_token(field_name id, std::string value);
        ~header_token();
    
    private:
        case_string m_field_name_;
        std::string m_field_value_;
        
        bool _M_valid_value(const std::string&) const;
    
    public:
        inline const case_string& name() const noexcept
        { return m_field_name_; }
        inline const std::string& value() const noexcept
        { return m_field_value_; }
    };
}}

#endif
