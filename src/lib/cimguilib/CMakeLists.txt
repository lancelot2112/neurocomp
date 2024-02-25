add_library(usegui STATIC)

target_compile_definitions(usegui PRIVATE 
	IMGUI_IMPL_API=extern\ \"C\"
	IMGUI_IMPL_OPENGL_LOADER_GL3W)

target_sources(usegui
PRIVATE
    usegui.c
    cimgui/cimgui.cpp
    cimgui/imgui/imgui.cpp
    cimgui/imgui/imgui_demo.cpp
    cimgui/imgui/imgui_draw.cpp
    cimgui/imgui/imgui_tables.cpp
    cimgui/imgui/imgui_widgets.cpp
    cimgui/imgui/backends/imgui_impl_glfw.cpp
    cimgui/imgui/backends/imgui_impl_opengl3.cpp
PUBLIC
    usegui.h
)

target_include_directories(usegui
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/cimgui
    ${CMAKE_CURRENT_LIST_DIR}/cimgui/generator/output
    ${CMAKE_CURRENT_LIST_DIR}/cimgui/imgui
    ${CMAKE_CURRENT_LIST_DIR}/cimgui/imgui/backends)
#target_compile_definitions(usegui PRIVATE PUBLIC -DCIMGUI_USE_OPENGL3 -DCIMGUI_USE_GLFW "-DCIMGUI_FREETYPE=1")
#target_link_libraries(usegui PRIVATE useglfw freetype)
target_compile_definitions(usegui PRIVATE PUBLIC -DCIMGUI_USE_OPENGL3 -DCIMGUI_USE_GLFW)
target_link_libraries(usegui PRIVATE useglfw)