/**
 * Created by TekuConcept on December 28, 2018
 */

#ifndef _IMPACT_HTTP_RESPONSE_MESSAGE_H_
#define _IMPACT_HTTP_RESPONSE_MESSAGE_H_

#include <string>
#include <memory>
#include "rfc/http/TX/message.h"

namespace impact {
namespace http {
    class response_message : public message {
    public:
        virtual ~response_message();
        
        static std::shared_ptr<response_message>
            create(int code, std::string status);
        
        int code() const noexcept;
        const std::string& status() const noexcept;
        
    private:
        int m_code_;
        std::string m_status_;
        
        response_message();
    };
}}

#endif
