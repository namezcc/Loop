#cmake_minimum_required(VERSION 3.1)
set(ProjectName "Login")
project(Login)


include_directories(
${PROJECT_SOURCE_DIR}/layer 
${PROJECT_SOURCE_DIR}/module 
${PROJECT_SOURCE_DIR}/../Common/module 
${PROJECT_SOURCE_DIR}/../Common/layer 
)

file(GLOB_RECURSE SOUCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEAD_FILES ${PROJECT_SOURCE_DIR}/*.h)

file(GLOB_RECURSE COM_RECURSE ${PROJECT_SOURCE_DIR}/../Common/*.*)
file(GLOB_RECURSE PB_RECURSE ${PROJECT_SOURCE_DIR}/../../protoPB/client/*.*)

set(SQL_DATA ${PROJECT_SOURCE_DIR}/../sqldata/GameReflectData.cpp)

if(WIN32)
set(Loginlib
Crypto.lib
libprotobufd.lib
)
endif()

add_library(${ProjectName} SHARED ${SOUCE_FILES} ${HEAD_FILES} ${COM_RECURSE} ${PB_RECURSE} ${SQL_DATA})

target_link_libraries(${ProjectName} ${LoopLib} ${Loginlib})

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