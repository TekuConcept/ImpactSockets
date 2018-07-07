/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef _GENERIC_H_
#define _GENERIC_H_

#include "Environment.h"
#include <string>
#include <locale>

namespace Impact {
namespace Internal {

#if defined(__WINDOWS__)
    std::string toNarrowString(const wchar_t* original, char unknown = '?',
        const std::locale& env = std::locale());
	std::string getWinErrorMessage(unsigned long);
#endif

    // int getLastError();
	std::string getErrorMessage();
    // std::string getHostErrorMessage();

}}

#endif