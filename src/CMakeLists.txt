include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.9
)

FetchContent_MakeAvailable(glfw)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

#find_package( OpenGL REQUIRED )

add_executable(BinaryAI main.c)

add_subdirectory(hdvec)
add_subdirectory(memory)
add_subdirectory(nodes)
add_subdirectory(linmath)
add_subdirectory(lib)
target_link_libraries(BinaryAI PUBLIC hdvec memory nodes glfwsupport)