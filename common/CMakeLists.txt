add_library(Common STATIC)

add_subdirectory(utils)

target_link_libraries(Common PRIVATE fmt)