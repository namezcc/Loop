#cmake_minimum_required(VERSION 3.1)
set(ProjectName "Server")
project(Server)

include_directories(
${PROJECT_SOURCE_DIR}/ 
)

if(WIN32)
include_directories(
#E:/git/breakpad/src/ 
)

link_directories(
E:/git/breakpad/src/client/windows/Debug/lib 
)

set(Serverlib 
jsoncpp.lib
#exception_handler.lib
#crash_generation_client.lib
common.lib
)
else()
include_directories(
#/usr/local/include/breakpad/
)

#breakpad_client.a

set(Serverlib 
boost_filesystem
boost_system
pthread
dl
)
endif()

file(GLOB_RECURSE source ${PROJECT_SOURCE_DIR}/*.cpp ${PROJECT_SOURCE_DIR}/*.h)

add_executable(${ProjectName} ${source})
target_link_libraries(${ProjectName} ${LoopLib} ${Serverlib})

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