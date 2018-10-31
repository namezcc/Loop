#cmake_minimum_required(VERSION 3.1)
set(ProjectName "BattleScene")
project(BattleScene)

include_directories(
${PROJECT_SOURCE_DIR}/module 
)
file(GLOB_RECURSE SCENE_SOUCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE SCENE_HEAD_FILES ${PROJECT_SOURCE_DIR}/*.h)

if(WIN32)
set(Scenelib
Lua.lib 
serverpb.lib
libprotobufd.lib
)
else()
set(Scenelib )
endif()

add_library(${ProjectName} SHARED ${SCENE_SOUCE_FILES} ${SCENE_HEAD_FILES})
target_link_libraries(${ProjectName} ${LoopLib} ${Scenelib})

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