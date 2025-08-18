cmake_minimum_required(VERSION 2.6)
project(secs)

#set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
#set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/bin)
#set(LIBRARY_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/lib)

#enable_testing()
#include(CTest)
#include(TestHelpers)
#add_subdirectory(module/xstf)

list( APPEND CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs")


# SET SOURCES AND STUFF
file(GLOB_RECURSE SECS_SOURCES src/lib/*)

#set include dirs
set(SECS_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/
)

add_library(secs ${SECS_SOURCES})
target_include_directories(secs PUBLIC ${SECS_INCLUDE_DIRS})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src/lib FILES ${SECS_SOURCES})

#file(GLOB tests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/test/*.cpp)

#foreach(test ${tests})
#	get_filename_component(testName ${test} NAME_WE)
#	addTests(${testName} ${test} "src/lib;module/xstf/src" "secscore;xstf")
#endforeach()
