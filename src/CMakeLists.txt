add_executable(BinaryAI main.c)

add_subdirectory(hdvec)
add_subdirectory(memory)
add_subdirectory(nodes)
target_link_libraries(BinaryAI PUBLIC hdvec memory nodes)