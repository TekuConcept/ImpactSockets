/**
 * Created by TekuConcept on July 15, 2018
 */

#include "sockets/impact_error.h"

#include <ctime>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "sockets/environment.h"

#if defined(__WINDOWS__)
    #define STACK_DEPTH 62
  	#include <windows.h>
    #if !defined(IMPACT_WIN_NODEBUG)
      	#include <Dbghelp.h>
      	#pragma comment(lib, "Dbghelp.lib")
    #endif
#else
    #define STACK_DEPTH 100
    #include <execinfo.h>
    #include <cxxabi.h>
    #include <cstring>
#endif

using namespace impact;

impact_error::impact_error(const std::string& __arg)
: std::runtime_error(__arg)
{
	m_trace_ = _M_trace();

	std::ostringstream os;
	os << __arg << std::endl;
	os << m_trace_ << std::endl;
	m_what_.assign(os.str());
}


impact_error::~impact_error() throw()
{}


const char*
impact_error::what() const throw()
{
	return m_what_.c_str();
}


const char*
impact_error::message() const throw()
{
	return std::runtime_error::what();
}


const char*
impact_error::trace() const throw()
{
	return m_trace_.c_str();
}


#if defined(__WINDOWS__)

std::string
impact_error::_M_trace() const throw()
{
	try {
		std::ostringstream out;
		void *stack[STACK_DEPTH];

#if !defined(IMPACT_WIN_NODEBUG)
		HANDLE process = GetCurrentProcess();

		SYMBOL_INFO* symbol;
		symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(TCHAR), 1);
		symbol->MaxNameLen = 255;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

		if (process == NULL) {
			free(symbol);
			return "No Symbols";
		}

		SymInitialize(process, NULL, TRUE);
#endif /* IMPACT_WIN_NODEBUG */

		auto size = CaptureStackBackTrace(2, STACK_DEPTH, stack, NULL);

		out << "Trace: ";
#if defined(IMPACT_WIN_NODEBUG)
		out << "(no debug symbols)";
#endif /* IMPACT_WIN_NODEBUG */
		out << std::endl;

		for (unsigned int i = 0; i < size; i++) {
#if !defined(IMPACT_WIN_NODEBUG)
			SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
			out << "\t(" << symbol->Name << ": " << std::hex << (int)stack[i] << ") [";
			out << std::hex << symbol->Address << "]" << std::endl;
#else
			out << "\t(" << std::hex << stack[i] << ") [?]" << std::endl;
#endif /* IMPACT_WIN_NODEBUG */
		}

#if !defined(IMPACT_WIN_NODEBUG)
		free(symbol);
#endif /* IMPACT_WIN_NODEBUG */

		return out.str();
	}
	catch (...) {
		return "stack trace error";
	}
}

#else /* Clang | GCC */

std::string
impact_error::_M_trace() const throw()
{
	try {
		std::ostringstream out;
		void* stack[STACK_DEPTH];

		/* WARNING: backtrace is async-signal-unsafe */
		size_t size = backtrace(stack, STACK_DEPTH);
		char** symbols = backtrace_symbols(stack, size);

		if (symbols == NULL)
			return "No Symbols";

		out << "Trace: " << std::endl;
		for (size_t i = 0; i < size; i++) {
			std::string token(symbols[i]);
			out << _M_demangle(token) << std::endl;
		}

		free(symbols);
		return out.str();
	}
	catch (...) {
		return "stack trace error";
	}
}

#endif /* __WINDOWS__ */


std::string
impact_error::_M_demangle(std::string __token) const throw()
{
#if !defined(__WINDOWS__)
	try {
		auto start = __token.find_first_of('(') + 1;
		auto end = __token.find_first_of('+', start);

		if (start == std::string::npos ||
			end == std::string::npos ||
			start >= end) {
			// DMSG("[StackTrace] No Symbol for " << token);
			return __token;
		}

		int status = 0;
		unsigned int size = end - start;
		auto token_name = __token.substr(start, size);
		auto fname = (char*)malloc(size);
		std::memset(fname, '\0', size);
		auto mc_str = abi::__cxa_demangle(
			&token_name[0],
			fname,
			&size,
			&status
		);

		if (status == 0) {
			std::string dname(mc_str);
			free(mc_str); /* mc_str = fname */
			return "(" + dname + __token.substr(end);
		}
	}
	catch (...) { /* do nothing - just return __token */ }
#endif
	return __token;
}
