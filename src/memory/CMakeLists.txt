add_library(memory "")

target_sources(memory
PRIVATE
    mem_bundles.c)

target_include_directories(memory
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})