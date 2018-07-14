/**
 * Created by TekuConcept on July 12, 2018
 */

#include "sockets/async_error.h"

using namespace impact;

async_error::async_error(const std::string& __arg)
: std::runtime_error(__arg)
{}


async_error::~async_error() throw()
{}


const char*
async_error::what() const throw()
{
  return std::runtime_error::what();
}
