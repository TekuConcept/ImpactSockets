/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef _IMPACT_IO_ERROR_H_
#define _IMPACT_IO_ERROR_H_

#include <string>
#include <exception>

namespace impact {
  class io_error : public std::exception {
  public:
    explicit io_error(const std::string& arg);
    virtual ~io_error() throw();
    virtual const char* what() const throw();

  private:
    std::string m_msg_;
  };
}

#endif