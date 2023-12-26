add_executable(BinaryAI main.c)

add_subdirectory(hdvec)
add_subdirectory(memory)
target_link_libraries(BinaryAI PRIVATE hdvec memory)