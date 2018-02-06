#cmake_minimum_required(VERSION 3.1)
set(ProjectName "Console")

project(Console)

set(test_lib
Loop.lib
)

file(GLOB_RECURSE CON_SOUCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE CON_HEAD_FILES ${PROJECT_SOURCE_DIR}/*.h)

add_executable(${ProjectName} ${CON_HEAD_FILES} ${CON_SOUCE_FILES})
target_link_libraries(${ProjectName} ${test_lib})

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