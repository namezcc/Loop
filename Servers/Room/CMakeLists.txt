#cmake_minimum_required(VERSION 3.1)
set(ProjectName "Room")
project(Room)


include_directories(
${PROJECT_SOURCE_DIR} 
${PROJECT_SOURCE_DIR}/layer 
${PROJECT_SOURCE_DIR}/module 
${PROJECT_SOURCE_DIR}/../Common/module 
${PROJECT_SOURCE_DIR}/../Common/layer 
)

file(GLOB_RECURSE SOUCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEAD_FILES ${PROJECT_SOURCE_DIR}/*.h)

set(COM_RECURSE 
#${PROJECT_SOURCE_DIR}/../Common/module/ProxyNodeModule.cpp 
#${PROJECT_SOURCE_DIR}/../Common/module/SendProxyDbModule.cpp 
)

file(GLOB_RECURSE OBJECT_RECURSE ${PROJECT_SOURCE_DIR}/../struct/*.cpp)

set(SQL_DATA ${PROJECT_SOURCE_DIR}/../sqldata/GameReflectData.cpp)

add_library(${ProjectName} SHARED ${SOUCE_FILES} ${HEAD_FILES} ${COM_RECURSE} ${SQL_DATA} ${OBJECT_RECURSE})

if(WIN32)
set(DBProxylib
Crypto.lib
clientpb.lib
serverpb.lib
libprotobufd.lib
)
endif()

target_link_libraries(${ProjectName} ${LoopLib} ${DBProxylib})

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