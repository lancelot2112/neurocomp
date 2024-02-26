add_subdirectory(glfwlib)
add_subdirectory(guilib)
add_subdirectory(dialoglib)
add_subdirectory(blaslib)

add_library(thirdparty INTERFACE)

target_link_libraries(thirdparty
    INTERFACE
        useglfw
        usegui
        usedialog
        useblas
)