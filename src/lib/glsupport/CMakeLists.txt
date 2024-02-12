add_library(glsupport STATIC)

target_sources(glsupport
PRIVATE
    shader_load.c
    glad.c
PUBLIC
    glsupport.h
    glad.h
)

target_include_directories(glsupport
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(glsupport PUBLIC glfw)