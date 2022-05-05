add_library(Common SHARED)

add_subdirectory(utils)

target_link_libraries(Common PRIVATE fmt)

# Add target to build dir
add_custom_command(
	TARGET Common POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/common/Common.dll
		${FINAL_DIR}/Common.dll
)