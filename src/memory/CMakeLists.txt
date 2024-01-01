add_library(memory STATIC)

target_sources(memory
PRIVATE
    membndl.c
PUBLIC
    membndl.h
)

target_include_directories(memory  
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(memory PUBLIC hdvec)