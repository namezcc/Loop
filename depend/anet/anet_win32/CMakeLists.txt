aux_source_directory(. DIR_SRCS)
add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
add_library(anet_win64 STATIC anet_win32.c)
set_target_properties( anet_win64 PROPERTIES OUTPUT_NAME_DEBUG "anet_win64")
set_target_properties( anet_win64 PROPERTIES PREFIX "")

#	ARCHIVE_OUTPUT_DIRECTORY ${NFOutPutDir}
#	RUNTIME_OUTPUT_DIRECTORY ${NFOutPutDir}
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
set(OutPutDir ${PROJECT_SOURCE_DIR}/../Build/Debug)
else()
set(OutPutDir ${PROJECT_SOURCE_DIR}/../Build/Release)
endif()
set_target_properties( anet_win64 PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${OutPutDir}
	RUNTIME_OUTPUT_DIRECTORY ${OutPutDir}
	LIBRARY_OUTPUT_DIRECTORY ${OutPutDir} 
)