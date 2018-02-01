CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

OPTION(BUILD_SHARED_LIBS "Build jsoncpp_lib as a shared library." OFF)

if(BUILD_SHARED_LIBS)
set(library SHARED)
else()
set(library STATIC)
endif()

add_subdirectory(anet)