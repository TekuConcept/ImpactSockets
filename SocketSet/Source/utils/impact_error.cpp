/**
 * Created by TekuConcept on July 15, 2018
 */

#include "utils/impact_error.h"

#include <ctime>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <memory>

#define DMSG(x) std::cout << x << std::endl

#include "utils/environment.h"

#if defined(__OS_WINDOWS__)
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
    #if defined(__OS_LINUX__)
        #define STACK_OFFSET 2
    #else
        #define STACK_OFFSET 3
    #endif
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

#if defined(__OS_WINDOWS__)

std::string
impact_error::_M_trace() const throw()
{
    try {
        std::ostringstream out;
        void* stack[STACK_DEPTH];

#if !defined(IMPACT_WIN_NODEBUG)
        HANDLE process = GetCurrentProcess();
        if (process == NULL)
            return "No Symbols";

        std::shared_ptr<SYMBOL_INFO> symbol(
            (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(TCHAR), 1),
            [](SYMBOL_INFO* ptr) {
                free(ptr);
            }
        );
        symbol->MaxNameLen = 255;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);


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
            SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol.get());
            out << "\t(" << symbol->Name << ": ";
            out << std::hex << (int)stack[i] << ") [";
            out << std::hex << symbol->Address << "]" << std::endl;
#else
            out << "\t(" << std::hex << stack[i] << ") [?]" << std::endl;
#endif /* IMPACT_WIN_NODEBUG */
        }

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

        // /* WARNING: backtrace is async-signal-unsafe */
        size_t size = backtrace(stack, STACK_DEPTH);
        char** raw_symbols = backtrace_symbols(stack, size);
        if (raw_symbols == NULL)
            return "No Symbols";
        
        std::shared_ptr<char*> symbols(
            raw_symbols,
            [](char** ptr) { free(ptr); }
        );

        out << "Trace: " << std::endl;
        for (size_t i = STACK_OFFSET; i < size; i++) {
            std::string token(raw_symbols[i]);
            out << _M_demangle(token) << std::endl;
        }

        return out.str();
    }
    catch (...) {
        return "stack trace error";
    }
    return "no symbols";
}

#endif /* __OS_WINDOWS__ */


std::string
impact_error::_M_demangle(std::string __token) const throw()
{
#if !defined(__OS_WINDOWS__)
    try {
    #if defined(__OS_LINUX__)
        // GCC Linux symbols are in the form
        // "(symbol+offset) address"
        auto start = __token.find_first_of('(') + 1;
        auto end   = __token.find_first_of('+', start);
    #else /* __OS_APPLE__ */
        // Apple clang symbols are in the form
        // # bin address symbol + offset
        auto start = __token.find(" _") + 1;
        auto end   = __token.find(" +");
    #endif

        if (start == std::string::npos ||
            end == std::string::npos ||
            start >= end) {
            return __token;
        }

        int status = 0;
        size_t fname_size = end - start;
        auto token_name = __token.substr(start, fname_size);
        
        char* fname = (char*)malloc(fname_size); // has to be malloc'd
        if (fname == NULL)
            return __token;
        
        char* mc_str = abi::__cxa_demangle(
            &token_name[0],
            fname, // realloc'd internally
            &fname_size,
            &status
        ); // mc_str = fname
        
        std::string dname;
        if (mc_str == NULL) {
            free(fname);
            return __token;
        }
        else {
            dname.assign(mc_str);
            free(mc_str);
        }

        if (status == 0) {
        #if defined(__OS_LINUX__)
            return "(" + dname + __token.substr(end);
        #else
            return __token.substr(0, start) + dname + __token.substr(end);
        #endif
        }
    }
    catch (...) { /* do nothing - just return __token */ }
#endif
    return __token;
}
