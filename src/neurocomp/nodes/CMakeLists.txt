target_sources(neurocomp
PRIVATE
    node.c
    connect.c
    #node_predictive.c
    neuroplot.cpp
PUBLIC
    node.h
    connect.h
)

target_include_directories(neurocomp  
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(neurocomp PRIVATE thirdparty)