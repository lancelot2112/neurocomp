include(${CMAKE_CURRENT_LIST_DIR}/embed.cmake)

## embed_resource("data.dat" "data.h" "DATA")
embed_resource("triangle.vs" "triangle_shaders.h" "vsTriangle")
embed_resource("vert_color.fs" "triangle_shaders.h" "fsVertColor")