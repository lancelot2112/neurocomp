target_sources(neurocomp
PRIVATE
    node.c
    nodeout.c
    node_predictive.c
    neuroplot.cpp
PUBLIC
    node.h
    nodeout.h
)

target_include_directories(neurocomp  
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(neurocomp PRIVATE thirdparty)