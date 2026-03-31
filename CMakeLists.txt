cmake_minimum_required(VERSION 3.16)
project(yuki)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(libs/utf8proc)
add_subdirectory(src)
add_subdirectory(tests)
