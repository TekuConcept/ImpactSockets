/**
 * Created by TekuConcept on December 28, 2018
 */

#ifndef _IMPACT_HTTP_HEADER_H_
#define _IMPACT_HTTP_HEADER_H_

#include <iostream>
#include <string>

#include "rfc/http/types.h"
#include "utils/case_string.h"

namespace impact {
namespace http {
    // caching object
    // (speed up execution by reducing validation checks)
    class header_t {
    public:
        header_t(std::string line);
        header_t(std::string name, std::string value);
        header_t(field_name id, std::string value);
        ~header_t();

        std::string to_string() const;

        inline const case_string& name() const noexcept
        { return m_field_name_; }
        inline const std::string& value() const noexcept
        { return m_field_value_; }

        friend std::ostream& operator<<(std::ostream&, const header_t&);

    private:
        case_string m_field_name_;
        std::string m_field_value_;
        
        bool _M_valid_value(const std::string&) const;
    };
}}

#endif
