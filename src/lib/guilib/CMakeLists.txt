add_library(usegui STATIC)

target_sources(usegui
PRIVATE
    usegui.cpp
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    implot/implot.cpp
    implot/implot_items.cpp
    implot/implot_demo.cpp
PUBLIC
    usegui.h
)

target_include_directories(usegui
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/imgui
    ${CMAKE_CURRENT_LIST_DIR}/imgui/backends
    ${CMAKE_CURRENT_LIST_DIR}/implot)

target_compile_definitions(usegui PRIVATE PUBLIC)
target_link_libraries(usegui PRIVATE useglfw)