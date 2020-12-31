/**
 * Created by TekuConcept on July 15, 2018
 */

#ifndef _IMPACT_IMPACT_ERROR_H_
#define _IMPACT_IMPACT_ERROR_H_

#include <string>
#include <stdexcept>

namespace impact {
    class impact_error : public std::runtime_error {
    public:
        explicit impact_error(const std::string& arg);
        virtual ~impact_error() throw();
        virtual const char* what() const throw();
        virtual const char* message() const throw();
        virtual const char* trace() const throw();
    protected:
        std::string m_what_;
        std::string m_trace_;

        std::string _M_trace() const throw();
        std::string _M_demangle(std::string) const throw();
    };
}

#endif
