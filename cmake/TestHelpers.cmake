
# use "item1;item2;item3;item4" syntax to pass several files directly

function(addTests testName testFiles includes libraries)

	add_executable(${testName} ${testFiles})
	include_directories(${testName} ${includes})
	target_link_libraries(${testName} ${libraries})
	add_test(NAME ${testName} COMMAND $<TARGET_FILE:${testName}>)

endfunction(addTests)
