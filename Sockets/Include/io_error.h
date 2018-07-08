/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef _IO_ERROR_H_
#define _IO_ERROR_H_

#include <string>
#include <exception>

namespace Impact {
  class io_error : public std::exception {
    std::string _M_msg;
  public:
    /** Takes a character string describing the error.  */
    explicit io_error(const std::string&  __arg);

    virtual ~io_error() throw();

    /** Returns a C-style character string describing the general cause of
     *  the current error (the same string passed to the ctor).  */
    virtual const char* what() const throw();
  };
}

#endif
