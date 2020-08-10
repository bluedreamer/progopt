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
        argsy.h
        argsy/cmdline.h
        argsy/config.h
        argsy/environment_iterator.h
        argsy/eof_iterator.h
        argsy/errors.h
        argsy/option.h
        argsy/options_description.h
        argsy/parsers.h
        argsy/positional_options.h
        argsy/value_semantic.h
        argsy/variables_map.h
        argsy/version.h
        argsy/detail/cmdline.h
        argsy/detail/config_file.h
        argsy/detail/convert.h
        argsy/detail/parsers.h
        argsy/detail/utf8_codecvt_facet.h
        argsy/detail/value_semantic.h
        )

add_executable(progopt main.cpp)

add_subdirectory(example)
add_subdirectory(test)
