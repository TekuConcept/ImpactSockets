/**
 * Created by TekuConcept on December 28, 2018
 */

#ifndef _IMPACT_HTTP_HEADER_H_
#define _IMPACT_HTTP_HEADER_H_

#include <iostream>
#include <string>
#include <vector>

#include "rfc/http/types.h"
#include "utils/case_string.h"

namespace impact {
namespace http {
    
    class header_t {
    public:
        header_t(std::string line);
        header_t(std::string name, std::string value);
        header_t(field_name id, std::string value); // recommended
        ~header_t();

        std::string to_string() const;
        void append_value(std::string value);
        void append_value_from(const header_t& header);

        inline const case_string& name() const noexcept
        { return m_field_name_; }
        inline const std::string& value() const noexcept
        { return m_field_value_; }

        void name(std::string);
        void value(std::string);

        friend std::ostream& operator<<(std::ostream&, const header_t&);

    private:
        case_string m_field_name_;
        std::string m_field_value_;

        // used to optimize certain checks
        // should not be publicly accessible
        bool m_describes_body_size_;
        field_name m_field_name_id_;

        bool _M_valid_value(const std::string&) const;

        friend class header_list;
        friend class message_t;
        friend class message_builder;
    };

}}

#endif
