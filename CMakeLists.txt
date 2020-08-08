cmake_minimum_required(VERSION 3.17)
project(progopt)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

include_directories(".")
add_executable(progopt
        main.cpp
        program_options.h
        options_description.h
        positional_options.h
        parsers.h
        variables_map.h
        cmdline.h
        errors.h
        option.h
        value_semantic.h
        version.h
        cmdline.cpp
        convert.cpp
        parsers.cpp
        split.cpp
        value_semantic.cpp
        winmain.cpp
        config_file.cpp
        options_description.cpp
        positional_options.cpp
        utf8_codecvt_facet.cpp
        variables_map.cpp
        environment_iterator.h
        eof_iterator.h
        utf8_codecvt_facet.h
        utf8_codecvt_facet.cpp

        )
