target_sources(neurocomp
PRIVATE
    membndl.c
PUBLIC
    membndl.h
)

target_include_directories(neurocomp  
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

