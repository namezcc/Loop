#cmake_minimum_required(VERSION 3.1)
set(ProjectName "MatchState")
project(MatchState)

include_directories(
${PROJECT_SOURCE_DIR}/module 
)
file(GLOB_RECURSE PROXY_SOUCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE PROXY_HEAD_FILES ${PROJECT_SOURCE_DIR}/*.h)

if(WIN32)
set(MatchStatelib 
serverpb.lib
libprotobufd.lib
)
else()
set(MatchStatelib )
endif()

add_library(${ProjectName} SHARED ${PROXY_HEAD_FILES} ${PROXY_SOUCE_FILES})
target_link_libraries(${ProjectName} ${LoopLib} ${MatchStatelib})

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