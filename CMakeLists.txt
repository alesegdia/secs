cmake_minimum_required(VERSION 3.15)

project(secs)

#set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
#set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/bin)
#set(LIBRARY_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/lib)

option(BUILD_GTESTS "Build GTest unit tests" OFF)
option(BUILD_BENCHMARKS "Build Google Benchmark targets" OFF)

enable_testing()
include(CTest)
#include(TestHelpers)
#add_subdirectory(module/xstf)

if(BUILD_GTESTS)
    find_package(GTest REQUIRED)
endif()
if(BUILD_BENCHMARKS)
    find_package(benchmark REQUIRED)
endif()

# SET SOURCES AND STUFF
file(GLOB_RECURSE SECS_SOURCES src/lib/*)

#set include dirs
set(SECS_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/
)

add_library(secs ${SECS_SOURCES})
target_include_directories(secs PUBLIC ${SECS_INCLUDE_DIRS})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src/lib FILES ${SECS_SOURCES})

set_target_properties(secs PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Tests
if(BUILD_GTESTS)
    file(GLOB_RECURSE SECS_TEST_SOURCES src/gtests/*.cpp)

    add_executable(secs-tests ${SECS_TEST_SOURCES})
    target_link_libraries(secs-tests PRIVATE secs GTest::gtest GTest::gtest_main GTest::gmock)
    set_target_properties(secs-tests PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )
    add_test(NAME secs-tests COMMAND secs-tests)
endif()

# Benchmarks
if(BUILD_BENCHMARKS)
    file(GLOB_RECURSE SECS_BENCH_SOURCES src/benchmarks/*.cpp)

    add_executable(secs-benchmarks ${SECS_BENCH_SOURCES})
    target_link_libraries(secs-benchmarks PRIVATE secs benchmark::benchmark)
    set_target_properties(secs-benchmarks PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )
endif()

#file(GLOB tests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/test/*.cpp)

#foreach(test ${tests})
#	get_filename_component(testName ${test} NAME_WE)
#	addTests(${testName} ${test} "src/lib;module/xstf/src" "secscore;xstf")
#endforeach()
