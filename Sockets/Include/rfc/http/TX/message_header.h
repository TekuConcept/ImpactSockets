/**
 * Created by TekuConcept on December 28, 2018
 */

#ifndef _IMPACT_HTTP_MESSAGE_HEADER_H_
#define _IMPACT_HTTP_MESSAGE_HEADER_H_

#include <string>
#include "utils/case_string.h"

namespace impact {
namespace http {
    class message_header {
    public:
        message_header(std::string name, std::string value);
        ~message_header();
        const std::string& field_name() const noexcept;
        const std::string& field_value() const noexcept;
    
    private:
        std::string m_field_name_;
        std::string m_field_value_;
    };
}}

#endif
