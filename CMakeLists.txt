cmake_minimum_required(VERSION 3.17)
project(progopt)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

include_directories(${PROJECT_SOURCE_DIR})
set(LIBRARY_NAME "libprogopt")

add_library(${LIBRARY_NAME} src/cmdline.cpp
        src/config_file.cpp
        src/convert.cpp
        src/options_description.cpp
        src/parsers.cpp
        src/positional_options.cpp
        src/split.cpp
        src/utf8_codecvt_facet.cpp
        src/value_semantic.cpp
        src/variables_map.cpp
        src/winmain.cpp
        program_options.hpp
        program_options/cmdline.hpp
        program_options/config.hpp
        program_options/environment_iterator.hpp
        program_options/eof_iterator.hpp
        program_options/errors.hpp
        program_options/option.hpp
        program_options/options_description.hpp
        program_options/parsers.hpp
        program_options/positional_options.hpp
        program_options/value_semantic.hpp
        program_options/variables_map.hpp
        program_options/version.hpp
        program_options/detail/cmdline.hpp
        program_options/detail/config_file.hpp
        program_options/detail/convert.hpp
        program_options/detail/parsers.hpp
        program_options/detail/utf8_codecvt_facet.hpp
        program_options/detail/value_semantic.hpp
        )

add_executable(progopt main.cpp)

add_subdirectory(example)
add_subdirectory(test)
