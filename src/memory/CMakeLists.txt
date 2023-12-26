add_library(memory "")

target_sources(memory
PRIVATE
    mem_hashed.c)

target_include_directories(memory
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})