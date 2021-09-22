#cmake_minimum_required(VERSION 3.1)
set(ProjectName "MysqlServer")
project(MysqlServer)


include_directories(
${PROJECT_SOURCE_DIR}/layer 
${PROJECT_SOURCE_DIR}/module 
)

file(GLOB_RECURSE SOUCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEAD_FILES ${PROJECT_SOURCE_DIR}/*.h)
set(SQL_DATA ${PROJECT_SOURCE_DIR}/../sqldata/GameReflectData.cpp
${PROJECT_SOURCE_DIR}/../Common/proto_sql.cpp
)

file(GLOB_RECURSE OBJECT_RECURSE ${PROJECT_SOURCE_DIR}/../struct/*.cpp)

add_library(${ProjectName} SHARED ${SOUCE_FILES} ${HEAD_FILES} ${SQL_DATA} ${OBJECT_RECURSE})

if(WIN32)
set(mysqllib
Crypto.lib
clientpb.lib
serverpb.lib
libprotobufd.lib
mysqlpp_d.lib
)
else()
set(mysqllib
serverpb.so
clientpb.so
Crypto.a
)
endif()

target_link_libraries(${ProjectName} ${LoopLib} ${mysqllib})

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