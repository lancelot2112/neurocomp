include(${CMAKE_CURRENT_LIST_DIR}/../embed.cmake)

## embed_resource("data.dat" "data.h" "DATA")
file(WRITE "shadercode.h" "#ifndef SHADERCODE_H\n") # clear file
file(APPEND "shadercode.h" "#define SHADERCODE_H\n")
embed_resource("triangle.vs" "shadercode.h" "vsTriangle")
embed_resource("vert_color.fs" "shadercode.h" "fsVertColor")
file(APPEND "shadercode.h" "#endif\n")

target_sources(resources
PRIVATE
    shader_load.c
PUBLIC
    shaders.h
)

target_include_directories(resources
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(resources PUBLIC thirdparty)