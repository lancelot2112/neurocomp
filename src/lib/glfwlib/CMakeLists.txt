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

add_library(useglfw STATIC)

target_sources(useglfw
PRIVATE
    glad.c
PUBLIC
    useglfw.h
    glad.h
)

target_include_directories(useglfw
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(useglfw PUBLIC glfw)