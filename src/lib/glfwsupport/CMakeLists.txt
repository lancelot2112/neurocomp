add_library(glfwsupport STATIC)

target_sources(glfwsupport
PRIVATE
    shader_load.c
    glad.c
PUBLIC
    glfwsupport.h
    glad.h
)

target_include_directories(glfwsupport
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(glfwsupport PUBLIC glfw)