add_library(Loader SHARED main.cpp "hookManager.h" "hookManager.cpp")

#set_target_properties(Loader PROPERTIES CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries(Loader PRIVATE Common fmt PolyHook_2)

# Add target to build dir
add_custom_command(
	TARGET Loader POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/loader/Loader.dll
		${FINAL_DIR}/loaders/Loader.dll
)