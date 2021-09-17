CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(serverpb)

file(GLOB_RECURSE source ${PROJECT_SOURCE_DIR}/*.*)

add_library(serverpb ${library} ${source})
target_link_libraries(serverpb ${lib_link})
set_target_properties( serverpb PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${OutPutDir}
	RUNTIME_OUTPUT_DIRECTORY ${OutPutDir}
	LIBRARY_OUTPUT_DIRECTORY ${OutPutDir} 
)

target_compile_definitions(serverpb
 PRIVATE 	LIBPROTOC_EXPORTS
)