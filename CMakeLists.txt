cmake_minimum_required(VERSION 3.10)

add_compile_options(-std=c++17)
add_compile_options(-g)

project(sgrep)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/test/)

add_executable(sgrep ${PROJECT_SOURCE_DIR}/src/sgrep.cpp)

target_include_directories(sgrep PUBLIC ${PROJECT_SOURCE_DIR}/include/)