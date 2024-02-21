add_library(glfwlib STATIC)

target_sources(glfwlib
PRIVATE
    glad.c
PUBLIC
    glfw.h
    glad.h
)

target_include_directories(glfwlib
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(glfwlib PUBLIC glfw)