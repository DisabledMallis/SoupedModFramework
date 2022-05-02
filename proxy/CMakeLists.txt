add_library(wininet SHARED main.cpp)

target_link_libraries(wininet PRIVATE fmt)

# Add target to build dir
add_custom_command(
	TARGET wininet POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/proxy/${CMAKE_CONFIGURATION_TYPES}/wininet.dll
		${FINAL_DIR}/proxies/wininet.dll
)