cmake_minimum_required(VERSION 2.6)
project(secs)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/lib)

enable_testing()
include(CTest)
include(TestHelpers)

add_subdirectory(module/xstf)

list( APPEND CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs")


# SET SOURCES AND STUFF
file(GLOB_RECURSE LIB_SRC src/lib/*)

add_library(secscore ${LIB_SRC})

file(GLOB tests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/test/*.cpp)

foreach(test ${tests})
	get_filename_component(testName ${test} NAME_WE)
	addTests(${testName} ${test} "lib;module/xstf/include" "secscore;xstf")
endforeach()
