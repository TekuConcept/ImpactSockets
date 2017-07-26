/**
 * Created by TekuConcept on July 26, 2017
 */

#include "RFC/String.h"

using namespace Impact;

bool RFC2616::operator==(RFC2616::string lhs, std::string rhs) {
    string temp(rhs.c_str(), rhs.length());
    return temp == lhs;
}

bool RFC2616::operator==(std::string lhs, RFC2616::string rhs) {
    string temp(lhs.c_str(), lhs.length());
    return temp == rhs;
}