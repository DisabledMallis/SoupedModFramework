add_library(Loader SHARED)

target_sources(Loader PRIVATE
	"hookManager.cpp"
	"main.cpp"
)

add_subdirectory(dumper)
add_subdirectory(patcher)
add_subdirectory(scripts)

set(SDK_ROOT "${CMAKE_BINARY_DIR}/SDK/")
set(ULTRALIGHT_INCLUDE_DIR "${SDK_ROOT}/include")
set(ULTRALIGHT_BINARY_DIR "${SDK_ROOT}/bin")
set(ULTRALIGHT_RESOURCES_DIR "${SDK_ROOT}/resources")
set(ULTRALIGHT_INSPECTOR_DIR "${SDK_ROOT}/inspector")
set(ULTRALIGHT_LIBRARY_DIR "${SDK_ROOT}/lib")

target_include_directories(Loader PRIVATE "${ULTRALIGHT_INCLUDE_DIR}")
target_link_directories(Loader PRIVATE "${ULTRALIGHT_LIBRARY_DIR}")
target_link_libraries(Loader PRIVATE Common fmt PolyHook_2 SoupSDK glfw opengl32 ChakraCore)

# Add target to build dir
add_custom_command(
	TARGET Loader POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/loader/Loader.dll
		${FINAL_DIR}/loaders/Loader.dll
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_LIST_DIR}/scripts
		${FINAL_DIR}/scripts
)