add_executable(BinaryAI main.cpp)

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_compile_options($<$<COMPILE_LANG_AND_ID:C,GNU>:-Werror=implicit-function-declaration>
                    -Werror=return-type
                    -Werror=uninitialized
)

add_link_options(-lrt)

add_subdirectory(lib)
add_subdirectory(resources)
add_subdirectory(neurocomp)

target_link_libraries(BinaryAI PUBLIC neurocomp thirdparty resources)