/**
 * Created by TekuConcept on July 7, 2018
 */

#include "io_error.h"

using namespace Impact;

io_error::io_error(const std::string& __arg)
  : exception(), _M_msg(__arg) { }


io_error::~io_error() { }


const char* io_error::what() const { return _M_msg.c_str(); }
