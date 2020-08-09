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
        argsy/cmdline.hpp
        argsy/config.hpp
        argsy/environment_iterator.hpp
        argsy/eof_iterator.hpp
        argsy/errors.hpp
        argsy/option.hpp
        argsy/options_description.hpp
        argsy/parsers.hpp
        argsy/positional_options.hpp
        argsy/value_semantic.hpp
        argsy/variables_map.hpp
        argsy/version.hpp
        argsy/detail/cmdline.hpp
        argsy/detail/config_file.hpp
        argsy/detail/convert.hpp
        argsy/detail/parsers.hpp
        argsy/detail/utf8_codecvt_facet.hpp
        argsy/detail/value_semantic.hpp
        )

add_executable(progopt main.cpp)

add_subdirectory(example)
add_subdirectory(test)
