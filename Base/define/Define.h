#ifndef DEFINE_H
#define DEFINE_H
#include <stdio.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include <stdlib.h>

#define ASSERT(expr)                                      \
 do {                                                     \
  if (!(expr)) {                                          \
    fprintf(stderr,                                       \
            "Assertion failed in %s on line %d: %s\n",    \
            __FILE__,                                     \
            __LINE__,                                     \
            #expr);                                       \
    abort();                                              \
  }                                                       \
 } while (0)

#ifdef LOOP_MAKE_DLL
	#define LOOP_EXPORT __declspec(dllexport)
#elif define LOOP_STATIC
	#define LOOP_EXPORT
#else
	#define LOOP_EXPORT __declspec(dllimport)
#endif


#define MAX_CONN 5000

#define TOSTR(S) #S

#define SHARE std::shared_ptr

#endif