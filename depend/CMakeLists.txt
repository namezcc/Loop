CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(Depends)

OPTION(BUILD_SHARED_LIBS "Build jsoncpp_lib as a shared library." OFF)

if(BUILD_SHARED_LIBS)
set(library SHARED)
else()
set(library STATIC)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
set(OutPutDir ${PROJECT_SOURCE_DIR}/Build/Debug)
else()
set(OutPutDir ${PROJECT_SOURCE_DIR}/Build/Release)
endif()

add_subdirectory(anet)
add_subdirectory(Crypto)