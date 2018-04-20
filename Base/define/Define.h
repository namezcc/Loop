#ifndef DEFINE_H
#define DEFINE_H
#include <stdio.h>

#define PLATFORM_WIN 1
#define PLATFORM_LINUX 2

#if defined( __WIN32__ ) || defined( _WIN32 ) || defined(_WINDOWS) || defined(WIN) || defined(_WIN64) || defined( __WIN64__ )
#define PLATFORM PLATFORM_WIN
#else
#define PLATFORM PLATFORM_LINUX
#endif

#define EXPORT extern "C"  __declspec(dllexport)

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
  #elif define LOOP_STATIC
    #define LOOP_EXPORT
  #else
    #define LOOP_EXPORT __declspec(dllimport)
  #endif
#else
  #define LOOP_EXPORT
#endif

#define DLL_START_NAME DllStart

#define MAX_CONN 5000

#define TOSTR(S) TOSTR1(S)
#define TOSTR1(S) #S

#define SHARE std::shared_ptr

#if PLATFORM == PLATFORM_WIN
#define GET_UV_SOCKET(tcp) tcp->socket
#else
#define GET_UV_SOCKET(tcp) uv__stream_fd(tcp)
#endif // PLAT


//pack head size
#define PACK_HEAD_SIZE 12

#endif