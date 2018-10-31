#cmake_minimum_required(VERSION 3.1)
set(ProjectName "LoopProxy")
project(LoopProxy)

include_directories(
${PROJECT_SOURCE_DIR}/module 
)
file(GLOB_RECURSE PROXY_SOUCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE PROXY_HEAD_FILES ${PROJECT_SOURCE_DIR}/*.h)

set(COM_RECURSE ${PROJECT_SOURCE_DIR}/../Common/module/NoticeServerModule.cpp)

if(WIN32)
set(Proxylib
serverpb.lib
libprotobufd.lib
)
else()
set(Proxylib )
endif()

add_library(${ProjectName} SHARED ${PROXY_HEAD_FILES} ${PROXY_SOUCE_FILES} ${COM_RECURSE})
target_link_libraries(${ProjectName} ${LoopLib} ${Proxylib})

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