add_library(nodes STATIC)

target_sources(nodes
PRIVATE
    node.c
    #node_predictive.c
PUBLIC
    node.h
)

target_include_directories(nodes  
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})