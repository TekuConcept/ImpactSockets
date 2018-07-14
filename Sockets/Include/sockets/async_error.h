/**
 * Created by TekuConcept on July 12, 2018
 */

#ifndef _IMPACT_ASYNC_ERROR_H_
#define _IMPACT_ASYNC_ERROR_H_

#include <string>
#include <stdexcept>

namespace impact {
  class async_error : public std::runtime_error {
  public:
    explicit async_error(const std::string& arg);
    virtual ~async_error() throw();
    virtual const char* what() const throw();
  };
}

#endif
