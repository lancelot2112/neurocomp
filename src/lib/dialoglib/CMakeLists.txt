add_library(usedialog STATIC)

target_sources(usedialog
PRIVATE
    usedialog.c
PUBLIC
    usedialog.h
)

target_include_directories(usedialog
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})