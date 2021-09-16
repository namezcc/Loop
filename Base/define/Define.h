#ifndef DEFINE_H
#define DEFINE_H
#include <stdio.h>
#include <stdint.h>

#define PLATFORM_WIN 1
#define PLATFORM_LINUX 2

#if defined( __WIN32__ ) || defined( _WIN32 ) || defined(_WINDOWS) || defined(WIN) || defined(_WIN64) || defined( __WIN64__ )
#define PLATFORM PLATFORM_WIN
#else
#define PLATFORM PLATFORM_LINUX
#endif

#if PLATFORM == PLATFORM_WIN
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

#if PLATFORM == PLATFORM_WIN
  #ifdef LOOP_MAKE_DLL
    #define LOOP_EXPORT __declspec(dllexport)
  #elif LOOP_STATIC
    #define LOOP_EXPORT
  #else
    #define LOOP_EXPORT __declspec(dllimport)
  #endif

  #define EXPORT extern "C"  __declspec(dllexport)
#else
  #define LOOP_EXPORT
  #define EXPORT extern "C" __attribute ((visibility("default")))
#endif

#define DLL_START_NAME DllStart

#define MAX_CONN 5000
#define MAX_CLIENT_CONN 20000

#define TOSTR(S) TOSTR1(S)
#define TOSTR1(S) #S

#define SHARE std::shared_ptr

//pack head size
#define PACK_HEAD_SIZE 12

#endif