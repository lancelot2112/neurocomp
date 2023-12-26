cmake_minimum_required(VERSION 3.15)
project(BinaryAI VERSION 1.0 LANGUAGES C)
set(CMAKE_C_STANDARD 11)


add_subdirectory(src)

enable_testing()
add_subdirectory(test)

