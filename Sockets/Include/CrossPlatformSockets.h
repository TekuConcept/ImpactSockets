/**
* Created by TekuConcept on July 8, 2017
*/

#ifndef CROSS_PLATFORM_SOCKETS_H
#define CROSS_PLATFORM_SOCKETS_H

#if defined(_WIN32) | defined(_WIN64)
  #define ENV_WINDOWS
#elif defined(__linux__) | defined(__linux) | defined(linux) | defined(unix)
  #define ENV_LINUX
#endif

#ifdef ENV_WINDOWS
  #pragma warning (disable : 4514)
  #pragma warning (disable : 4710)
  #pragma warning (disable : 4626)
  #pragma warning (disable : 4996)
#endif

#ifndef API_DECLSPEC
	#ifdef ENV_WINDOWS
		#ifdef DLL_EXPORTS
			#define API_DECLSPEC __declspec(dllexport)
		#elif defined(STATIC)
			#define API_DECLSPEC
		#else
			#define API_DECLSPEC __declspec(dllimport)
		#endif
	#else
    #define API_DECLSPEC
	#endif // ENV_WINDOWS
#endif // API_DECLSPEC

#endif // CROSS_PLATFORM_H
