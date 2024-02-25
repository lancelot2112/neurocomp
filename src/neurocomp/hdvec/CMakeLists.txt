target_sources(neurocomp
PRIVATE
    binvec.c
PUBLIC
    binvec.h)

target_include_directories(neurocomp
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})