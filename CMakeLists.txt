cmake_minimum_required(VERSION 3.1)
project(Loop)

include_directories(
${PROJECT_SOURCE_DIR} 
${PROJECT_SOURCE_DIR}/Base/ 
${PROJECT_SOURCE_DIR}/Base/define/ 
${PROJECT_SOURCE_DIR}/Base/kernel/ 
${PROJECT_SOURCE_DIR}/Base/layer/ 
${PROJECT_SOURCE_DIR}/Base/module/ 
${PROJECT_SOURCE_DIR}/Base/utils/ 
${PROJECT_SOURCE_DIR}/librarys/libuv/include/ 
${PROJECT_SOURCE_DIR}/librarys/protobuf/include/ 
)

link_directories(
${PROJECT_SOURCE_DIR}/librarys/libs/win/ 
${PROJECT_SOURCE_DIR}/build/release/
)

set(lib_link
libuv.lib
)

file(GLOB_RECURSE SOUCE_FILES ${PROJECT_SOURCE_DIR}/Base/*.cpp)
file(GLOB_RECURSE HEAD_FILES ${PROJECT_SOURCE_DIR}/Base/*.h)

#aux_source_directory(${PROJECT_SOURCE_DIR}/Base/ SOUCE_FILES)

#message(STATUS "souce file" ${SOUCE_FILES})
#message(STATUS "head file" ${HEAD_FILES})

add_library(Loop STATIC ${HEAD_FILES} ${SOUCE_FILES})
target_link_libraries(Loop ${lib_link})

add_subdirectory(Test)