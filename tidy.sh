clang-tidy -p build --checks="-*,bugprone-*" --fix-errors src/*.cpp example/*.cpp test/*.cpp argsy/*.hpp argsy/detail/*.hpp
