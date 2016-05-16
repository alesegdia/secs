
include(TestHelpers)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

# SET SOURCES AND STUFF
file(GLOB_RECURSE LIB_SRC lib/*)

add_library(secscore ${LIB_SRC})

file(GLOB tests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test/*.cpp)

foreach(test ${tests})
	get_filename_component(testName ${test} NAME_WE)
	addTests(${testName} ${test} lib secscore)
endforeach()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/lib)

