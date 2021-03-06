#cmake_minimum_required(VERSION 3.1)
set(ProjectName "Master")
project(Master)

include_directories(
${PROJECT_SOURCE_DIR}/module 
)

file(GLOB_RECURSE Master_SOUCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE Master_HEAD_FILES ${PROJECT_SOURCE_DIR}/*.h)

set(SQL_DATA ${PROJECT_SOURCE_DIR}/../sqldata/ReflectData.cpp)

if(WIN32)
set(Masterlib
Crypto.lib
libprotobufd.lib
mysqlpp_d.lib
)
endif()

add_library(${ProjectName} SHARED ${Master_HEAD_FILES} ${Master_SOUCE_FILES} ${SQL_DATA})
target_link_libraries(${ProjectName} ${LoopLib} ${Masterlib})

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
-DRAPIDJSON_HAS_STDSTRING
)