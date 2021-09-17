CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
project(protoPB)
#add_compile_options(-std=c++11)

set(library SHARED)
#set(library STATIC)
set(CMAKE_BUILD_TYPE "Debug")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
set(OutPutDir ${PROJECT_SOURCE_DIR}/../_out/Debug)
else()
set(OutPutDir ${PROJECT_SOURCE_DIR}/../_out/Release)
endif()

if(WIN32)
include_directories(
${PROJECT_SOURCE_DIR}/../librarys/include/protobuf/ 
)
endif()

link_directories(
	${PROJECT_SOURCE_DIR}/../_out/Debug/ 
	${PROJECT_SOURCE_DIR}/../_out/ 
	${PROJECT_SOURCE_DIR}/../librarys/libs/win/Debug/ 
)

if(WIN32)
set(lib_link
libprotobufd.lib
)
else()
set(lib_link
protobuf
)
endif()

add_definitions(
-DPROTOBUF_USE_DLLS
)

add_subdirectory(client)
add_subdirectory(server)