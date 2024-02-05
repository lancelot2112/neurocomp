add_library(hdvec STATIC)

target_sources(hdvec
PRIVATE
    binvec.c
PUBLIC
    binvec.h
    linmath.h)

target_include_directories(hdvec
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})