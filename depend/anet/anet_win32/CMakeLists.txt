aux_source_directory(. DIR_SRCS)
add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
add_library(anet STATIC anet_win32.c)
#set_target_properties( anet_win64 PROPERTIES OUTPUT_NAME_DEBUG "anet_win64")
set_target_properties( anet PROPERTIES PREFIX "")

set_target_properties( anet PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${OutPutDir}
	RUNTIME_OUTPUT_DIRECTORY ${OutPutDir}
	LIBRARY_OUTPUT_DIRECTORY ${OutPutDir} 
)