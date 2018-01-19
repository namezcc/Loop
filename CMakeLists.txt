cmake_minimum_required(VERSION 3.1)
project(Loop)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

set(LPOutDir ${PROJECT_SOURCE_DIR}/_out/)
set(LPBaseOut ${PROJECT_SOURCE_DIR}/librarys/libs/win/)

include_directories(
${PROJECT_SOURCE_DIR} 
${PROJECT_SOURCE_DIR}/Base/ 
${PROJECT_SOURCE_DIR}/Base/define/ 
${PROJECT_SOURCE_DIR}/Base/kernel/ 
${PROJECT_SOURCE_DIR}/Base/layer/ 
${PROJECT_SOURCE_DIR}/Base/module/ 
${PROJECT_SOURCE_DIR}/Base/utils/ 
${PROJECT_SOURCE_DIR}/librarys/libuv/include/ 
${PROJECT_SOURCE_DIR}/librarys/protobuf/include/ 
${PROJECT_SOURCE_DIR}/librarys/jsoncpp/include/ 
)

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
	link_directories(
	${PROJECT_SOURCE_DIR}/librarys/libs/win/Debug/
	)
else()
	link_directories(
	${PROJECT_SOURCE_DIR}/librarys/libs/win/Release/
	)
endif()

set(lib_link
libuv.lib
libprotobuf.lib
jsoncpp.lib
)

file(GLOB_RECURSE SOUCE_FILES ${PROJECT_SOURCE_DIR}/Base/*.cpp)
file(GLOB_RECURSE HEAD_FILES ${PROJECT_SOURCE_DIR}/Base/*.h)
file(GLOB_RECURSE PROTO_FILES ${PROJECT_SOURCE_DIR}/protoPB/*.*)

#aux_source_directory(${PROJECT_SOURCE_DIR}/Base/ SOUCE_FILES)

#message(STATUS "souce file" ${SOUCE_FILES})
#message(STATUS "head file" ${HEAD_FILES})

add_library(Loop STATIC ${HEAD_FILES} ${SOUCE_FILES} ${PROTO_FILES})
#target_link_libraries(Loop ${lib_link})
set_target_properties(Loop PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${LPBaseOut}
	RUNTIME_OUTPUT_DIRECTORY ${LPBaseOut}
	LIBRARY_OUTPUT_DIRECTORY ${LPBaseOut} )

add_subdirectory(Test)
add_subdirectory(proxy)