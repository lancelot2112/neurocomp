target_sources(neurocomp
PRIVATE
    spikenode.c
    spikeasm.c
    spikeplot.cpp
PUBLIC
    spikenode.h
    spikeasm.h
)

target_include_directories(neurocomp  
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(neurocomp PRIVATE thirdparty)