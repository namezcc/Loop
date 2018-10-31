CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(Lua)

file(GLOB_RECURSE source ${PROJECT_SOURCE_DIR}/*.c)

add_library(Lua STATIC ${source})

set_target_properties( Lua PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${OutPutDir}
	RUNTIME_OUTPUT_DIRECTORY ${OutPutDir}
	LIBRARY_OUTPUT_DIRECTORY ${OutPutDir} 
)

target_compile_definitions(Lua
PRIVATE   LUA_COMPAT_5_1
)