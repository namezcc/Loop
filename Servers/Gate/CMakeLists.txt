#cmake_minimum_required(VERSION 3.1)
set(ProjectName "Gate")
project(Gate)

include_directories(
${PROJECT_SOURCE_DIR}/module 
)
file(GLOB_RECURSE SOUCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEAD_FILES ${PROJECT_SOURCE_DIR}/*.h)

if(WIN32)
set(gatelib
serverpb.lib
clientpb.lib
libprotobufd.lib
)
else()
set(gatelib
serverpb.so
clientpb.so
)
endif()

add_library(${ProjectName} SHARED ${HEAD_FILES} ${SOUCE_FILES})
target_link_libraries(${ProjectName} ${LoopLib} ${gatelib})

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