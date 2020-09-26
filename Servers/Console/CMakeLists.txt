#cmake_minimum_required(VERSION 3.1)
set(ProjectName "Console")
project(Console)

file(GLOB_RECURSE CON_SOUCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE CON_HEAD_FILES ${PROJECT_SOURCE_DIR}/*.h)

add_library(${ProjectName} SHARED ${CON_HEAD_FILES} ${CON_SOUCE_FILES})

if(WIN32)
set(consollib
libprotobufd.lib
)
endif()

target_link_libraries(${ProjectName} ${LoopLib} ${consollib})

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