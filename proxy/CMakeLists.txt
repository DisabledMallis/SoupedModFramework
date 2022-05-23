add_library(Winhttp SHARED main.cpp)

target_link_libraries(Winhttp PRIVATE fmt)

# Add target to build dir
add_custom_command(
	TARGET Winhttp POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/proxy/Winhttp.dll
		${FINAL_DIR}/proxies/Winhttp.dll
)