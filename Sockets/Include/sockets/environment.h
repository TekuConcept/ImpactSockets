/**
* Created by TekuConcept on July 8, 2017
*/

#ifndef _IMPACT_ENVIRONMENT_H_
#define _IMPACT_ENVIRONMENT_H_

#if defined(_WIN32) | defined(_WIN64)
  #define __WINDOWS__
#elif defined(__linux__) | defined(__linux) | defined(linux)
  #define __LINUX__
/*
#else __APPLE__
*/
#endif

#ifdef __WINDOWS__
  #pragma warning (disable : 4514)
  #pragma warning (disable : 4710)
  #pragma warning (disable : 4626)
  #pragma warning (disable : 4996)
#endif

#ifndef SOC_DECLSPEC
	#ifdef __WINDOWS__
		#ifdef EXPORT
			#define SOC_DECLSPEC __declspec(dllexport)
		#elif defined(IMPORT)
			#define SOC_DECLSPEC __declspec(dllimport)
		#else /* STATIC */
			#define SOC_DECLSPEC
		#endif
	#else
    #define SOC_DECLSPEC
	#endif // __WINDOWS__
#endif // SOC_DECLSPEC

#ifndef UNUSED
  #define UNUSED(x) (void)x
#endif

#ifndef VALUE
  #define VALUE(p) (*p)
#endif

#endif // _ENVIRONMENT_H_
