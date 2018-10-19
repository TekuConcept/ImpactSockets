/**
 * Created by TekuConcept on July 8, 2017
 */

#ifndef _IMPACT_ENVIRONMENT_H_
#define _IMPACT_ENVIRONMENT_H_

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
                 -: BUILD DEFINE FLAGS :-

    IMPACT_NO_WSAESTR: Disables in-house WSA string lookup.
    The default string lookup will be FormatMessage, which
    doesn't always return a value for the given code. Note:
    this only applies for Windows builds.

    IMPACT_WIN_NODEBUG: Disables stacktrace information
    on Windows systems which use features from Dbghelp.lib.

    IMPACT_WIN_SECURE_REUSE: Enables 'exclusive address use'
    security for all sockets that need to reuse an address.

\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "configure.h"

#define UNUSED(x) (void)x
#define UNUSED_FUNCTION(x)
#define DEREF(p) (*p)

#if defined(HAS_FALLTHROUGH_ATTRIBUTE)
    #define FALLTHROUGH [[fallthrough]]
#else
    #define FALLTHROUGH
#endif

#ifdef __OS_WINDOWS__
    #pragma warning (disable : 4514)
    #pragma warning (disable : 4710)
    #pragma warning (disable : 4626)
    #pragma warning (disable : 4996)
#endif

#ifndef SOC_DECLSPEC
    #ifdef __OS_WINDOWS__
        #ifdef EXPORT
            #define SOC_DECLSPEC __declspec(dllexport)
        #elif defined(IMPORT)
            #define SOC_DECLSPEC __declspec(dllimport)
        #else /* STATIC */
            #define SOC_DECLSPEC
        #endif
    #else
        #define SOC_DECLSPEC
    #endif // __OS_WINDOWS__
#endif // SOC_DECLSPEC

#endif // _IMPACT_ENVIRONMENT_H_
