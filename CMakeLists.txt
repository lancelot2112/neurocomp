cmake_minimum_required(VERSION 3.15)
project(BinaryAI VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

#set_compiler_options(-Wall)

#set_compiler_options(--coverage -fprofile-arcs -ftest-coverage)

add_subdirectory(src)

enable_testing()
add_subdirectory(test)

