add_library(Loader SHARED main.cpp)

target_link_libraries(Loader PRIVATE Common fmt)

# Add target to build dir
add_custom_command(
	TARGET Loader POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/loader/${CMAKE_CONFIGURATION_TYPES}/Loader.dll
		${FINAL_DIR}/loaders/Loader.dll
)