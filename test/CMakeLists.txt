
add_executable(cmdline_test cmdline_test.cpp)
target_link_libraries(cmdline_test libprogopt)

add_executable(exception_test exception_test.cpp)
target_link_libraries(exception_test libprogopt)

add_executable(exception_txt_test exception_txt_test.cpp)
target_link_libraries(exception_txt_test libprogopt)

add_executable(optional_test optional_test.cpp)
target_link_libraries(optional_test libprogopt)

add_executable(options_description_test options_description_test.cpp)
target_link_libraries(options_description_test libprogopt)

add_executable(parsers_test parsers_test.cpp)
target_link_libraries(parsers_test libprogopt)

add_executable(positional_options_test positional_options_test.cpp)
target_link_libraries(positional_options_test libprogopt)

add_executable(quick quick.cpp)
target_link_libraries(quick libprogopt)

add_executable(required_test required_test.cpp)
target_link_libraries(required_test libprogopt)

add_executable(split_test split_test.cpp)
target_link_libraries(split_test libprogopt)

add_executable(test_convert test_convert.cpp)
target_link_libraries(test_convert libprogopt)

add_executable(unicode_test unicode_test.cpp)
target_link_libraries(unicode_test libprogopt)

add_executable(unrecognized_test unrecognized_test.cpp)
target_link_libraries(unrecognized_test libprogopt)

add_executable(variable_map_test variable_map_test.cpp)
target_link_libraries(variable_map_test libprogopt)

add_executable(winmain winmain.cpp)
target_link_libraries(winmain libprogopt)

