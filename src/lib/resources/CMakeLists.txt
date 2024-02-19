include(${CMAKE_CURRENT_LIST_DIR}/embed.cmake)

## embed_resource("data.dat" "data.h" "DATA")
file(WRITE "shaders.h" "") # clear file
embed_resource("triangle.vs" "shaders.h" "vsTriangle")
embed_resource("vert_color.fs" "shaders.h" "fsVertColor")