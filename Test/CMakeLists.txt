#cmake_minimum_required(VERSION 3.1)
set(ProjectName "Test")

project(Test)

set(test_lib
ws2_32.lib
psapi.lib
iphlpapi.lib
user32.lib
userenv.lib
Loop.lib
libuv.lib
)

file(GLOB_RECURSE TEST_SOUCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE TEST_HEAD_FILES ${PROJECT_SOURCE_DIR}/*.h)

add_executable(${ProjectName} ${TEST_HEAD_FILES} ${TEST_SOUCE_FILES})
target_link_libraries(${ProjectName} ${test_lib})

add_definitions(
-D_DEBUG
-D_CONSOLE
-DHAVE_STRUCT_TIMESPEC
-D_WIN32_WINNT=0x0600
-DWIN32_LEAN_AND_MEAN
)