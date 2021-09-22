#cmake_minimum_required(VERSION 3.1)
set(ProjectName "LoginLock")
project(LoginLock)

include_directories(
${PROJECT_SOURCE_DIR}/layer 
${PROJECT_SOURCE_DIR}/module 
)

file(GLOB_RECURSE SOUCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEAD_FILES ${PROJECT_SOURCE_DIR}/*.h)

add_library(${ProjectName} SHARED ${SOUCE_FILES} ${HEAD_FILES})

if(WIN32)
set(DBProxylib
Crypto.lib
serverpb.lib
libprotobufd.lib
mysqlpp_d.lib
)
else()
set(DBProxylib
serverpb.so
Crypto.a
)
endif()

target_link_libraries(${ProjectName} ${LoopLib} ${DBProxylib})

set_target_properties(${ProjectName} PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${LPOutDir}
	RUNTIME_OUTPUT_DIRECTORY ${LPOutDir}
	LIBRARY_OUTPUT_DIRECTORY ${LPOutDir} )

add_definitions(
-D_DEBUG
-D_CONSOLE
-DHAVE_STRUCT_TIMESPEC
-D_WIN32_WINNT=0x0600
-DWIN32_LEAN_AND_MEAN
)