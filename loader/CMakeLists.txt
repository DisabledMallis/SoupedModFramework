add_library(Loader SHARED)

target_sources(Loader PRIVATE
	"main.cpp"
	"hookManager.cpp"
 "patcher/patchers.h" "patcher/patchers.cpp")

add_subdirectory(dumper)

target_link_libraries(Loader PRIVATE Common fmt PolyHook_2 SoupSDK)

# Add target to build dir
add_custom_command(
	TARGET Loader POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/loader/Loader.dll
		${FINAL_DIR}/loaders/Loader.dll
)