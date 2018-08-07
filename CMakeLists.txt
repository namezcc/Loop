cmake_minimum_required(VERSION 3.1)
project(Loop)

add_compile_options(-std=c++11)

set(CMAKE_BUILD_TYPE "Debug")

set(LPOutDir ${PROJECT_SOURCE_DIR}/_out/)
set(LPBaseOut ${PROJECT_SOURCE_DIR}/_out/)

include_directories(
${PROJECT_SOURCE_DIR} 
${PROJECT_SOURCE_DIR}/Base/ 
${PROJECT_SOURCE_DIR}/Base/define/ 
${PROJECT_SOURCE_DIR}/Base/kernel/ 
${PROJECT_SOURCE_DIR}/Base/layer/ 
${PROJECT_SOURCE_DIR}/Base/module/ 
${PROJECT_SOURCE_DIR}/Base/utils/ 
${PROJECT_SOURCE_DIR}/librarys/include/ 
${PROJECT_SOURCE_DIR}/librarys/include/redis/ 
${PROJECT_SOURCE_DIR}/depend/ 
)

if(WIN32)
include_directories(
${PROJECT_SOURCE_DIR}/librarys/include/protobuf/ 
${PROJECT_SOURCE_DIR}/librarys/include/libuv/ 
${PROJECT_SOURCE_DIR}/librarys/include/jsoncpp/ 
${PROJECT_SOURCE_DIR}/librarys/include/mysql/ 
E:/git/boost_1_66_0/ 
)

link_directories(
E:/git/boost_1_66_0/bin/vc14/lib
)

endif()

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
	link_directories(
	${PROJECT_SOURCE_DIR}/librarys/libs/win/Debug/ 
	${PROJECT_SOURCE_DIR}/depend/Build/Debug/ 
	${PROJECT_SOURCE_DIR}/_out/Debug/ 
	)

	add_definitions(-DDEBUG)
else()
	link_directories(
	${PROJECT_SOURCE_DIR}/librarys/libs/win/Release/
	${PROJECT_SOURCE_DIR}/depend/Build/Release/ 
	${PROJECT_SOURCE_DIR}/_out/Release/ 
	)
endif()

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
set(lib_link
libuv.lib
libprotobufd.lib
jsoncpp.lib
mysqlpp_d.lib
Lua.lib
)
else()
set(lib_link
libuv.lib
libprotobuf.lib
jsoncpp.lib
mysqlpp_d.lib
Lua.lib
)
endif()

if(WIN32)
set(lib_link ${lib_link} anet.lib libboost_filesystem-vc141-mt-gd-x64-1_66.lib Lua.lib)
set(LoopLib Loop.lib)
else()
set(lib_link anet.a Lua.a boost_filesystem.a)
endif()

file(GLOB_RECURSE SOUCE_FILES ${PROJECT_SOURCE_DIR}/Base/*.cpp)
file(GLOB_RECURSE HEAD_FILES ${PROJECT_SOURCE_DIR}/Base/*.h)
file(GLOB_RECURSE PROTO_FILES ${PROJECT_SOURCE_DIR}/protoPB/server/*.*)

add_library(Loop SHARED ${HEAD_FILES} ${SOUCE_FILES} ${PROTO_FILES})
target_link_libraries(Loop ${lib_link})
set_target_properties(Loop PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${LPBaseOut}
	RUNTIME_OUTPUT_DIRECTORY ${LPBaseOut}
	LIBRARY_OUTPUT_DIRECTORY ${LPBaseOut} )
	
target_compile_definitions(Loop
 PRIVATE	LOOP_MAKE_DLL
 PRIVATE 	LIBPROTOC_EXPORTS
)

add_definitions(
-DPROTOBUF_USE_DLLS
-DMYSQLPP_MYSQL_HEADERS_BURIED
)

add_subdirectory(Servers)