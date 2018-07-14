/**
 * Created by TekuConcept on July 7, 2018
 */

#include "sockets/io_error.h"

using namespace impact;

io_error::io_error(const std::string& __arg)
: std::runtime_error(__arg)
{}


io_error::~io_error() throw()
{}


const char*
io_error::what() const throw()
{
  return std::runtime_error::what();
}
