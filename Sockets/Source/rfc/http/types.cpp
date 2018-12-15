/**
 * Created by TekuConcept on December 15, 2018
 */

#include "rfc/http/types.h"

using namespace impact;

http::start_line::start_line()
: type(message_type::REQUEST),
  method("GET"), target("/"),
  http_major(1), http_minor(1)
{}


http::start_line::~start_line()
{}
