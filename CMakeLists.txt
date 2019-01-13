cmake_minimum_required(VERSION 3.1)
project(Loop)

add_compile_options(-std=c++11)

set(CMAKE_BUILD_TYPE "Debug")

if(NOT WIN32)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4819")

endif()

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
${PROJECT_SOURCE_DIR}/protoPB/ 
${PROJECT_SOURCE_DIR}/Servers/sqldata/ 
${PROJECT_SOURCE_DIR}/Servers/struct/ 
${PROJECT_SOURCE_DIR}/Servers/Common/ 
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

else()
link_directories(
/usr/lib64/mysql/
)
endif()

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
	link_directories(
	${PROJECT_SOURCE_DIR}/librarys/libs/win/Debug/ 
	${PROJECT_SOURCE_DIR}/depend/Build/Debug/ 
	${PROJECT_SOURCE_DIR}/_out/Debug/ 
	${PROJECT_SOURCE_DIR}/_out/ 
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
set(LoopLib Loop.lib libuv.lib jsoncpp.lib)
else()
set(lib_link anet.a Lua.a boost_filesystem boost_context.a uv.a jsoncpp.a protobuf mysqlclient mysqlpp)
set(LoopLib Loop)
endif()

file(GLOB_RECURSE SOUCE_FILES ${PROJECT_SOURCE_DIR}/Base/*.cpp)
file(GLOB_RECURSE HEAD_FILES ${PROJECT_SOURCE_DIR}/Base/*.h)
file(GLOB_RECURSE PROTO_FILES ${PROJECT_SOURCE_DIR}/protoPB/base/*.*)

add_library(Loop STATIC ${HEAD_FILES} ${SOUCE_FILES} ${PROTO_FILES})
target_link_libraries(Loop ${lib_link})
set_target_properties(Loop PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${LPBaseOut}
	RUNTIME_OUTPUT_DIRECTORY ${LPBaseOut}
	LIBRARY_OUTPUT_DIRECTORY ${LPBaseOut} )
	
#target_compile_definitions(Loop
 #PRIVATE	LOOP_MAKE_DLL
 #PRIVATE 	LIBPROTOC_EXPORTS
#)

add_definitions(
-DLOOP_STATIC
-DPROTOBUF_USE_DLLS
-DMYSQLPP_MYSQL_HEADERS_BURIED
-DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
)

if(WIN32)
add_definitions(
-D_WINSOCK_DEPRECATED_NO_WARNINGS
-D_SCL_SECURE_NO_WARNINGS
-D_CRT_SECURE_NO_WARNINGS
)
endif()

add_subdirectory(Servers)