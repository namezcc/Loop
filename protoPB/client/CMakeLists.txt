CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(clientpb)

file(GLOB_RECURSE source ${PROJECT_SOURCE_DIR}/*.*)

add_library(clientpb ${library} ${source})
target_link_libraries(clientpb ${lib_link})
set_target_properties( clientpb PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${OutPutDir}
	RUNTIME_OUTPUT_DIRECTORY ${OutPutDir}
	LIBRARY_OUTPUT_DIRECTORY ${OutPutDir} 
)

target_compile_definitions(clientpb
 PRIVATE 	LIBPROTOC_EXPORTS
)